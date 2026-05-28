#include "ItemBase.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "PlayerCharacter.h"
#include "InventoryComponent.h"
#include "LootTableEntry.h"
#include "CKGameState.h"
#include "Engine/DataTable.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SetReplicateMovement(false);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetGenerateOverlapEvents(true);
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetMobility(EComponentMobility::Movable);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(Root);
	InteractionSphere->SetSphereRadius(150.0f);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	RarityLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RarityLight"));
	RarityLight->SetupAttachment(Root);
	RarityLight->SetIntensity(5000.0f);
	RarityLight->SetAttenuationRadius(250.0f);

	RarityColors.Add(EItemRarity::Common, FLinearColor::Green);
	RarityColors.Add(EItemRarity::Uncommon, FLinearColor::Blue);
	RarityColors.Add(EItemRarity::Rare, FLinearColor::FromSRGBColor(FColor(128, 0, 128)));
	RarityColors.Add(EItemRarity::Legendary, FLinearColor::Red);

	ItemType = EItemType::Melee;
	bUseCrosshair = false;
	GoldValue = 10;
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();
	UpdateRarityVisuals();
}

void AItemBase::Interact_Implementation(APlayerCharacter* Interactor)
{
    if (!HasAuthority() || !Interactor) return;

    UInventoryComponent* Inv = Interactor->GetInventoryComponent();
    if (Inv)
    {
        Inv->Server_TryPickupItem(this);
    }
}

FText AItemBase::GetInteractionText_Implementation() const
{
	// Data tables are the single source of truth for display info.
	// Look up the item name from the data table rather than reading ItemName directly.
	UWorld* World = GetWorld();
	if (World)
	{
		ACKGameState* GS = World->GetGameState<ACKGameState>();
		if (GS)
		{
			TArray<UDataTable*> Tables = GS->GetAllItemTables();
			UClass* MyClass = GetClass();
			FString MyPath = MyClass->GetPathName();

			for (UDataTable* Table : Tables)
			{
				if (!Table) continue;
				TArray<FLootTableEntry*> Rows;
				Table->GetAllRows<FLootTableEntry>(TEXT("InteractionPrompt"), Rows);
				for (auto* Row : Rows)
				{
					if (Row->ItemClass && (Row->ItemClass == MyClass || Row->ItemClass->GetPathName() == MyPath))
					{
						return FText::Format(NSLOCTEXT("CK", "PickupItem", "Pick up {0}"), Row->ItemName);
					}
				}
			}
		}
	}

	// Fallback if data table not available (shouldn't happen at runtime)
	return FText::Format(NSLOCTEXT("CK", "PickupItem", "Pick up {0}"), FText::FromName(ItemName));
}

void AItemBase::RestoreAuthoredMeshRelativeTransform()
{
	if (!Mesh)
	{
		return;
	}

	if (Mesh->GetAttachParent() != Root)
	{
		Mesh->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	}

	const AItemBase* DefaultItem = GetClass() ? GetClass()->GetDefaultObject<AItemBase>() : nullptr;
	if (DefaultItem && DefaultItem->Mesh)
	{
		Mesh->SetRelativeTransform(DefaultItem->Mesh->GetRelativeTransform());
	}
}

void AItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AItemBase, ItemType);
	DOREPLIFETIME(AItemBase, ItemName);
    DOREPLIFETIME(AItemBase, Description);
	DOREPLIFETIME(AItemBase, Rarity);
	DOREPLIFETIME(AItemBase, GoldValue);
}

void AItemBase::OnRep_Rarity()
{
	UpdateRarityVisuals();
}

void AItemBase::UpdateRarityVisuals()
{
	// Data tables are the single source of truth for display info.
	// Look up the correct rarity from the data table for the light color.
	EItemRarity DisplayRarity = Rarity;
	UWorld* World = GetWorld();
	if (World)
	{
		if (ACKGameState* GS = World->GetGameState<ACKGameState>())
		{
			UClass* MyClass = GetClass();
			FString MyPath = MyClass->GetPathName();
			for (UDataTable* Table : GS->GetAllItemTables())
			{
				if (!Table) continue;
				TArray<FLootTableEntry*> Rows;
				Table->GetAllRows<FLootTableEntry>(TEXT("RarityVisual"), Rows);
				for (auto* Row : Rows)
				{
					if (Row->ItemClass && (Row->ItemClass == MyClass || Row->ItemClass->GetPathName() == MyPath))
					{
						DisplayRarity = Row->Rarity;
						break;
					}
				}
				if (DisplayRarity != Rarity) break;
			}
		}
	}

	if (RarityColors.Contains(DisplayRarity))
	{
		FLinearColor Color = RarityColors[DisplayRarity];
		if (RarityLight)
		{
			RarityLight->SetLightColor(Color);
		}
	}
}

void AItemBase::OnPickedUp()
{
	if (Mesh)
	{
		Mesh->SetSimulatePhysics(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RestoreAuthoredMeshRelativeTransform();
	}

	if (InteractionSphere)
	{
		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (RarityLight)
	{
		RarityLight->SetVisibility(false);
	}

	Multicast_SetStoredState();
}

void AItemBase::OnDropped(FVector DropLocation)
{
	if (Mesh)
	{
		Mesh->SetSimulatePhysics(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RestoreAuthoredMeshRelativeTransform();
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (InteractionSphere)
	{
		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	if (RarityLight)
	{
		RarityLight->SetVisibility(true);
	}

	Multicast_SetDroppedState(DropLocation);
}

void AItemBase::ApplyLootTableEntry(const FLootTableEntry& Entry)
{
	SetItemName(FName(*Entry.ItemName.ToString()));
	SetDescription(Entry.Description);
	SetRarity(Entry.Rarity);
	SetGoldValue(Entry.SellPrice);
	UpdateRarityVisuals();
}

void AItemBase::Multicast_SetStoredState_Implementation()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void AItemBase::Multicast_SetDroppedState_Implementation(FVector DropLocation)
{
	if (Mesh)
	{
		Mesh->SetSimulatePhysics(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RestoreAuthoredMeshRelativeTransform();
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorLocation(DropLocation);
	
	if (Mesh)
	{
		Mesh->SetSimulatePhysics(true);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void AItemBase::Equip(USceneComponent* AttachParent, FName SocketName)
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (Mesh)
	{
		Mesh->SetSimulatePhysics(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RestoreAuthoredMeshRelativeTransform();
	}

	if (AttachParent && SocketName != NAME_None && AttachParent->DoesSocketExist(SocketName))
	{
		AttachToComponent(AttachParent, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	}

	if (InteractionSphere)
	{
		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (RarityLight)
	{
		RarityLight->SetVisibility(false);
	}
}

void AItemBase::Unequip()
{
	if (Mesh)
	{
		Mesh->SetSimulatePhysics(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RestoreAuthoredMeshRelativeTransform();
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}
