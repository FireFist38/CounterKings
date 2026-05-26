#include "InteractionComponent.h"
#include "Components/SphereComponent.h"
#include "ItemBase.h"
#include "PlayerCharacter.h"
#include "InventoryComponent.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	InteractionRadius = 200.0f;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Attach sphere to owner's root
	AActor* Owner = GetOwner();
	if (Owner)
	{
		InteractionSphere = NewObject<USphereComponent>(Owner, TEXT("InteractionSphere"));
		InteractionSphere->RegisterComponent();
		InteractionSphere->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		InteractionSphere->SetSphereRadius(InteractionRadius);
		InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		InteractionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	}
}

void UInteractionComponent::PerformInteraction()
{
	Server_PerformInteraction();
}

void UInteractionComponent::Server_PerformInteraction_Implementation()
{
	AItemBase* BestItem = GetBestInteractable();
	if (BestItem)
	{
		APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwner());
		if (Character && Character->GetInventoryComponent())
		{
			Character->GetInventoryComponent()->Server_TryPickupItem(BestItem);
		}
	}
}

AItemBase* UInteractionComponent::GetBestInteractable()
{
	if (!InteractionSphere) return nullptr;

	TArray<AActor*> OverlappingActors;
	InteractionSphere->GetOverlappingActors(OverlappingActors, AItemBase::StaticClass());

	AItemBase* ClosestItem = nullptr;
	float MinDistanceSq = FMath::Square(InteractionRadius + 100.0f);

	for (AActor* Actor : OverlappingActors)
	{
		AItemBase* Item = Cast<AItemBase>(Actor);
		if (Item)
		{
			float DistanceSq = FVector::DistSquared(GetOwner()->GetActorLocation(), Item->GetActorLocation());
			if (DistanceSq < MinDistanceSq)
			{
				MinDistanceSq = DistanceSq;
				ClosestItem = Item;
			}
		}
	}

	return ClosestItem;
}
