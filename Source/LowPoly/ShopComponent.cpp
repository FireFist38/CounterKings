#include "ShopComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "InventoryComponent.h"
#include "LootTableEntry.h"
#include "LootGenerationData.h"
#include "Engine/DataTable.h"

UShopComponent::UShopComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
    LockedSlots.Init(false, 5);
}

void UShopComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UShopComponent, CurrentShopPool);
    DOREPLIFETIME(UShopComponent, LockedSlots);
}

void UShopComponent::GenerateShopPool(int32 Round, float Luck)
{
	if (!GetOwner()->HasAuthority() || !ShopDataTable || !GenerationSettings) return;

    if (CurrentShopPool.Num() != 5) CurrentShopPool.Init(nullptr, 5);
    if (LockedSlots.Num() != 5) LockedSlots.Init(false, 5);

	TArray<FLootTableEntry*> AllItems;
	ShopDataTable->GetAllRows<FLootTableEntry>(TEXT("ShopItems"), AllItems);

	for (int32 i = 0; i < 5; i++)
	{
        if (!LockedSlots[i])
        {
		    int32 RandIndex = FMath::RandRange(0, AllItems.Num() - 1);
		    CurrentShopPool[i] = AllItems[RandIndex]->ItemClass;
        }
	}
	OnShopUpdated.Broadcast();
}

void UShopComponent::Server_PurchaseItem_Implementation(int32 SlotIndex)
{
	if (!CurrentShopPool.IsValidIndex(SlotIndex) || !CurrentShopPool[SlotIndex]) return;

	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwner());
	if (!Character || !Character->GetAttributeComponent() || !Character->GetInventoryComponent()) return;

	TSubclassOf<AItemBase> ItemClass = CurrentShopPool[SlotIndex];
	
	int32 ItemCost = 50; 
	TArray<FLootTableEntry*> AllItems;
	if (ShopDataTable)
	{
		ShopDataTable->GetAllRows<FLootTableEntry>(TEXT("ShopItems"), AllItems);
		for(auto* Entry : AllItems)
		{
			if(Entry->ItemClass == ItemClass)
			{
				ItemCost = Entry->BuyPrice;
				break;
			}
		}
	}
	
	if (Character->GetAttributeComponent()->GetGold() >= ItemCost)
	{
		Character->GetAttributeComponent()->AddGold(-ItemCost);
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AItemBase* NewItem = GetWorld()->SpawnActor<AItemBase>(ItemClass, Character->GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
		
		if (NewItem)
		{
			Character->GetInventoryComponent()->Server_TryPickupItem(NewItem);
			CurrentShopPool[SlotIndex] = nullptr;
            LockedSlots[SlotIndex] = false;
			OnShopUpdated.Broadcast();
		}
	}
}

void UShopComponent::Server_ToggleLockSlot_Implementation(int32 SlotIndex)
{
    if (LockedSlots.IsValidIndex(SlotIndex))
    {
        LockedSlots[SlotIndex] = !LockedSlots[SlotIndex];
        OnShopUpdated.Broadcast();
    }
}

void UShopComponent::Server_SellItem_Implementation(int32 InventoryIndex)
{
    APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwner());
    if (!Character || !Character->GetAttributeComponent() || !Character->GetInventoryComponent()) 
    {
        return;
    }

    TArray<AItemBase*> AllInventory = Character->GetInventoryComponent()->GetAllInventoryItems();

    if (!AllInventory.IsValidIndex(InventoryIndex) || !AllInventory[InventoryIndex]) 
    {
        return;
    }

    AItemBase* ItemToSell = AllInventory[InventoryIndex];

    int32 SellPrice = 25; 
    TArray<FLootTableEntry*> AllItems;
    if (ShopDataTable)
    {
        ShopDataTable->GetAllRows<FLootTableEntry>(TEXT("ShopItems"), AllItems);
        for(auto* Entry : AllItems)
        {
            if(Entry->ItemClass == ItemToSell->GetClass())
            {
                SellPrice = Entry->SellPrice;
                break;
            }
        }
    }

    Character->GetAttributeComponent()->AddGold(SellPrice);
    
    // Remove from inventory
    Character->GetInventoryComponent()->RemoveItem(ItemToSell);
    ItemToSell->Destroy(); 
    
    OnShopUpdated.Broadcast();
}

void UShopComponent::Server_RerollShop_Implementation()
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwner());
	if (Character && Character->GetAttributeComponent() && Character->GetAttributeComponent()->GetGold() >= RerollCost)
	{
		Character->GetAttributeComponent()->AddGold(-RerollCost);
		GenerateShopPool(1, Character->GetAttributeComponent()->Luck);
	}
}
