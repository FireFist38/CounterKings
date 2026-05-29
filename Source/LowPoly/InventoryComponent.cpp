#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "ItemBase.h"
#include "AbilityBase.h"
#include "PerkBase.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "MainHandBase.h"
#include "OffHandBase.h"
#include "ArmorBase.h"
#include "ConsumableBase.h"
#include "InventoryUpgradeComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	MainHandSlots.SetNum(2);
	OffHandSlots.SetNum(2);
	InventorySlots.SetNum(12);
    AbilitySlots.SetNum(3);
    PerkSlots.SetNum(10);
	ActiveMainHandIndex = 0;
	ActiveOffHandIndex = 0;
	ArmorSet = nullptr;
	ConsumableSlot = nullptr;

	MaxPickupCap = 10;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, MainHandSlots);
	DOREPLIFETIME(UInventoryComponent, OffHandSlots);
	DOREPLIFETIME(UInventoryComponent, InventorySlots);
    DOREPLIFETIME(UInventoryComponent, AbilitySlots);
    DOREPLIFETIME(UInventoryComponent, PerkSlots);
	DOREPLIFETIME(UInventoryComponent, ActiveMainHandIndex);
	DOREPLIFETIME(UInventoryComponent, ActiveOffHandIndex);
	DOREPLIFETIME(UInventoryComponent, ArmorSet);
	DOREPLIFETIME(UInventoryComponent, ConsumableSlot);
}

TArray<AItemBase*> UInventoryComponent::GetAllInventoryItems() const
{
	TArray<AItemBase*> AllItems;
	AllItems.Append(MainHandSlots);
	AllItems.Append(OffHandSlots);
	AllItems.Append(InventorySlots);
    for (AAbilityBase* Ability : AbilitySlots) if (Ability) AllItems.Add(Ability);
    for (APerkBase* Perk : PerkSlots) if (Perk) AllItems.Add(Perk);
	if (ArmorSet) AllItems.Add(ArmorSet);
	if (ConsumableSlot) AllItems.Add(ConsumableSlot);
	return AllItems;
}

AItemBase* UInventoryComponent::GetBagItem(int32 Index) const
{
	if (InventorySlots.IsValidIndex(Index))
		return InventorySlots[Index];
	return nullptr;
}

AItemBase* UInventoryComponent::GetActiveMainHandItem() const
{
	if (MainHandSlots.IsValidIndex(ActiveMainHandIndex))
		return MainHandSlots[ActiveMainHandIndex];
	return nullptr;
}

AItemBase* UInventoryComponent::GetActiveOffHandItem() const
{
	if (OffHandSlots.IsValidIndex(ActiveOffHandIndex))
		return OffHandSlots[ActiveOffHandIndex];
	return nullptr;
}

bool UInventoryComponent::IsOffHandLocked() const
{
	AMainHandBase* MainHand = Cast<AMainHandBase>(GetActiveMainHandItem());
	return MainHand && MainHand->IsTwoHanded();
}

bool UInventoryComponent::IsAbilitySlotUnlocked(int32 SlotIndex) const
{
    APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwner());
    if (!PC || !PC->GetAttributeComponent()) return false;

    int32 CurrentLevel = PC->GetAttributeComponent()->GetLevel();
    int32 Threshold = GetLevelThresholdForAbilitySlot(SlotIndex);

    return CurrentLevel >= Threshold;
}

int32 UInventoryComponent::GetLevelThresholdForAbilitySlot(int32 SlotIndex) const
{
    switch (SlotIndex)
    {
        case 0: return Ability1LevelThreshold;
        case 1: return Ability2LevelThreshold;
        case 2: return Ability3LevelThreshold;
        default: return 999;
    }
}

void UInventoryComponent::Server_CycleMainHand_Implementation()
{
	APlayerCharacter* OwningCharacter = Cast<APlayerCharacter>(GetOwner());
	if (!OwningCharacter || MainHandSlots.Num() <= 0)
	{
		return;
	}

	AItemBase* OldActiveItem = GetActiveMainHandItem();
	ActiveMainHandIndex = (ActiveMainHandIndex + 1) % MainHandSlots.Num();
	AItemBase* NewActiveItem = GetActiveMainHandItem();

    OwningCharacter->Multicast_SwapItem(OldActiveItem, NewActiveItem, OwningCharacter->GetMainHandSocketName());

	// Sync offhand visual to match two-handed state of the new main hand
	RefreshOffHandAttachment();
}

void UInventoryComponent::RefreshOffHandAttachment()
{
	APlayerCharacter* OwningCharacter = Cast<APlayerCharacter>(GetOwner());
	if (!OwningCharacter) return;

	AItemBase* OffHandItem = GetActiveOffHandItem();
	if (!OffHandItem) return;

	if (IsOffHandLocked())
	{
		// Two-handed main equipped — detach the offhand visually (stays in slot)
		OwningCharacter->Multicast_DetachItem(OffHandItem);
	}
	else
	{
		// One-handed (or no main) — make sure offhand is attached
		OwningCharacter->Multicast_AttachItem(OffHandItem, OwningCharacter->GetOffHandSocketName());
	}
}

void UInventoryComponent::Server_CycleOffHand_Implementation()
{
	if (IsOffHandLocked()) return;

	APlayerCharacter* OwningCharacter = Cast<APlayerCharacter>(GetOwner());
	if (!OwningCharacter || OffHandSlots.Num() <= 0)
	{
		return;
	}

	AItemBase* OldActiveItem = GetActiveOffHandItem();
    ActiveOffHandIndex = (ActiveOffHandIndex + 1) % OffHandSlots.Num();
	AItemBase* NewActiveItem = GetActiveOffHandItem();

    OwningCharacter->Multicast_SwapItem(OldActiveItem, NewActiveItem, OwningCharacter->GetOffHandSocketName());
}

void UInventoryComponent::Server_TryPickupItem_Implementation(AItemBase* Item)
{
    if (!Item) return;

    for (int32 i = 0; i < InventorySlots.Num(); i++)
    {
        if (InventorySlots[i] == nullptr)
        {
            InventorySlots[i] = Item;
            Item->SetOwner(GetOwner());
            Item->OnPickedUp();
            return;
        }
    }
}

void UInventoryComponent::Server_SwapItem_Implementation(AItemBase* NewItem, int32 SlotIndex)
{
}

void UInventoryComponent::Server_EquipItem_Implementation(AItemBase* Item, ESlotGroup Slot)
{
    if (!Item) return;

    APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwner());
    if (!PC) return;

    Item->SetOwner(PC);

    FName SocketName = NAME_None;
    if (Slot == ESlotGroup::MainHand) SocketName = PC->GetMainHandSocketName();
    else if (Slot == ESlotGroup::OffHand) SocketName = PC->GetOffHandSocketName();

    PC->Multicast_AttachItem(Item, SocketName);
}

void UInventoryComponent::Server_UnequipItem_Implementation(ESlotGroup Slot, int32 Index)
{
    AItemBase* ItemToUnequip = nullptr;
    
    if (Slot == ESlotGroup::MainHand && MainHandSlots.IsValidIndex(Index))
        ItemToUnequip = MainHandSlots[Index];
    else if (Slot == ESlotGroup::OffHand && OffHandSlots.IsValidIndex(Index))
        ItemToUnequip = OffHandSlots[Index];
    else if (Slot == ESlotGroup::Armor)
        ItemToUnequip = ArmorSet;

    if (ItemToUnequip)
    {
        APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwner());
        if (PC) PC->Multicast_DetachItem(ItemToUnequip);
        
        // Clear reference
        if (Slot == ESlotGroup::MainHand) MainHandSlots[Index] = nullptr;
        else if (Slot == ESlotGroup::OffHand) OffHandSlots[Index] = nullptr;
        else if (Slot == ESlotGroup::Armor) ArmorSet = nullptr;
    }
}

void UInventoryComponent::Server_MoveItem_Implementation(ESlotGroup FromGroup, int32 FromIndex, ESlotGroup ToGroup, int32 ToIndex)
{
	auto GetPtr = [this](ESlotGroup Group, int32 Index) -> AItemBase**
	{
		if (Group == ESlotGroup::MainHand && MainHandSlots.IsValidIndex(Index)) return &MainHandSlots[Index];
		if (Group == ESlotGroup::OffHand && OffHandSlots.IsValidIndex(Index)) return &OffHandSlots[Index];
		if (Group == ESlotGroup::Bag && InventorySlots.IsValidIndex(Index)) return &InventorySlots[Index];
        if (Group == ESlotGroup::Ability && AbilitySlots.IsValidIndex(Index)) return (AItemBase**)&AbilitySlots[Index];
        if (Group == ESlotGroup::Perk && PerkSlots.IsValidIndex(Index)) return (AItemBase**)&PerkSlots[Index];
		if (Group == ESlotGroup::Armor) return &ArmorSet;
		if (Group == ESlotGroup::Consumable) return &ConsumableSlot;
		return nullptr;
	};

	auto IsCompatibleWithGroup = [this](AItemBase* Item, ESlotGroup Group, int32 Index) -> bool
	{
		if (!Item) return true;

		switch (Group)
		{
		case ESlotGroup::MainHand:
			return Item->IsA(AMainHandBase::StaticClass());
		case ESlotGroup::OffHand:
			return Item->IsA(AOffHandBase::StaticClass()) && !IsOffHandLocked();
		case ESlotGroup::Armor:
			return Item->IsA(AArmorBase::StaticClass());
		case ESlotGroup::Consumable:
			return Item->IsA(AConsumableBase::StaticClass());
        case ESlotGroup::Ability:
            return Item->IsA(AAbilityBase::StaticClass()) && IsAbilitySlotUnlocked(Index);
        case ESlotGroup::Perk:
            return Item->IsA(APerkBase::StaticClass());
		case ESlotGroup::Bag:
			return true;
		default:
			return false;
		}
	};

	auto ApplyItemState = [this](AItemBase* Item, ESlotGroup DestinationGroup)
	{
		if (!Item) return;

		APlayerCharacter* OwningCharacter = Cast<APlayerCharacter>(GetOwner());
		if (!OwningCharacter)
		{
            Item->SetOwner(nullptr);
			Item->OnPickedUp();
			return;
		}

        Item->SetOwner(OwningCharacter);

		if (DestinationGroup == ESlotGroup::MainHand)
		{
			if (MainHandSlots.IsValidIndex(ActiveMainHandIndex) && MainHandSlots[ActiveMainHandIndex] == Item)
			{
				OwningCharacter->Multicast_AttachItem(Item, OwningCharacter->GetMainHandSocketName());
			}
			else
			{
				OwningCharacter->Multicast_DetachItem(Item);
			}
		}
		else if (DestinationGroup == ESlotGroup::OffHand)
		{
			if (OffHandSlots.IsValidIndex(ActiveOffHandIndex) && OffHandSlots[ActiveOffHandIndex] == Item)
			{
				OwningCharacter->Multicast_AttachItem(Item, OwningCharacter->GetOffHandSocketName());
			}
			else
			{
				OwningCharacter->Multicast_DetachItem(Item);
			}
		}
        else if (DestinationGroup == ESlotGroup::Perk)
        {
            if (APerkBase* Perk = Cast<APerkBase>(Item)) Perk->OnPerkActivated(OwningCharacter);
            OwningCharacter->Multicast_DetachItem(Item);
        }
		else
		{
			OwningCharacter->Multicast_DetachItem(Item);
		}
	};

    auto DeactivateItemState = [this](AItemBase* Item, ESlotGroup OldGroup)
    {
        if (!Item) return;
        APlayerCharacter* OwningCharacter = Cast<APlayerCharacter>(GetOwner());
        if (!OwningCharacter) return;

        if (OldGroup == ESlotGroup::Perk)
        {
            if (APerkBase* Perk = Cast<APerkBase>(Item)) Perk->OnPerkDeactivated(OwningCharacter);
        }
    };

	if (FromGroup == ToGroup && FromIndex == ToIndex)
	{
		return;
	}

	AItemBase** SourcePtr = GetPtr(FromGroup, FromIndex);
	AItemBase** TargetPtr = GetPtr(ToGroup, ToIndex);

	if (!SourcePtr || !TargetPtr)
	{
		return;
	}

	AItemBase* MovingItem = *SourcePtr;
	AItemBase* ExistingTargetItem = *TargetPtr;

	if (!MovingItem)
	{
		return;
	}

	if (!IsCompatibleWithGroup(MovingItem, ToGroup, ToIndex) || !IsCompatibleWithGroup(ExistingTargetItem, FromGroup, FromIndex))
	{
		return;
	}

    // Handle perk deactivation BEFORE swapping if leaving Perk group
    DeactivateItemState(MovingItem, FromGroup);
    DeactivateItemState(ExistingTargetItem, ToGroup);

	*SourcePtr = ExistingTargetItem;
	*TargetPtr = MovingItem;

	ApplyItemState(MovingItem, ToGroup);
	ApplyItemState(ExistingTargetItem, FromGroup);

	// Sync offhand visibility if either side touched a hand slot (handles dragging a 2H weapon in/out)
	if (FromGroup == ESlotGroup::MainHand || ToGroup == ESlotGroup::MainHand ||
		FromGroup == ESlotGroup::OffHand || ToGroup == ESlotGroup::OffHand)
	{
		RefreshOffHandAttachment();
	}
}

void UInventoryComponent::Server_DropItem_Implementation(ESlotGroup FromGroup, int32 FromIndex)
{
	auto GetPtr = [this](ESlotGroup Group, int32 Index) -> AItemBase**
	{
		if (Group == ESlotGroup::MainHand && MainHandSlots.IsValidIndex(Index)) return &MainHandSlots[Index];
		if (Group == ESlotGroup::OffHand && OffHandSlots.IsValidIndex(Index)) return &OffHandSlots[Index];
		if (Group == ESlotGroup::Bag && InventorySlots.IsValidIndex(Index)) return &InventorySlots[Index];
        if (Group == ESlotGroup::Ability && AbilitySlots.IsValidIndex(Index)) return (AItemBase**)&AbilitySlots[Index];
        if (Group == ESlotGroup::Perk && PerkSlots.IsValidIndex(Index)) return (AItemBase**)&PerkSlots[Index];
		if (Group == ESlotGroup::Armor) return &ArmorSet;
		if (Group == ESlotGroup::Consumable) return &ConsumableSlot;
		return nullptr;
	};

	AItemBase** ItemPtr = GetPtr(FromGroup, FromIndex);
	if (!ItemPtr || !*ItemPtr)
	{
		return;
	}

	AItemBase* ItemToDrop = *ItemPtr;
    
    // Deactivate if it was a perk
    if (FromGroup == ESlotGroup::Perk)
    {
        if (APerkBase* Perk = Cast<APerkBase>(ItemToDrop)) Perk->OnPerkDeactivated(Cast<APlayerCharacter>(GetOwner()));
    }

	*ItemPtr = nullptr;

    ItemToDrop->SetOwner(nullptr);

	if (FromGroup == ESlotGroup::MainHand || FromGroup == ESlotGroup::OffHand || FromGroup == ESlotGroup::Armor)
	{
		if (APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwner()))
		{
			PC->Multicast_DetachItem(ItemToDrop);
		}
	}

	const AActor* OwnerActor = GetOwner();
	const FVector DropLocation = OwnerActor
		? OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 100.0f
		: ItemToDrop->GetActorLocation();

	ItemToDrop->OnDropped(DropLocation);
}

void UInventoryComponent::Server_RequestUpgradeRarity_Implementation(AItemBase* Item, EItemUpgradeTarget UpgradeTarget)
{
    if (!Item) return;

    // 1. Validate ownership: Item must be in one of our slots
    bool bOwned = false;
    if (MainHandSlots.Contains(Item)) bOwned = true;
    else if (OffHandSlots.Contains(Item)) bOwned = true;
    else if (InventorySlots.Contains(Item)) bOwned = true;
    else if (ArmorSet == Item) bOwned = true;
    else if (ConsumableSlot == Item) bOwned = true;

    if (!bOwned) return;

    // 2. Get Upgrade Component
    APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwner());
    if (!Character) return;

    UInventoryUpgradeComponent* UpgradeComp = Character->FindComponentByClass<UInventoryUpgradeComponent>();
    if (!UpgradeComp) return;

    UAttributeComponent* Attr = Character->GetAttributeComponent();
    if (!Attr) return;

    // 3. Resolve target rarity and cost
    EItemRarity CurrentRarity = Item->Rarity;
    EItemRarity TargetRarity = UpgradeComp->ResolveTargetRarity(CurrentRarity, UpgradeTarget);

    int32 Cost = 0;
    if (!UpgradeComp->GetUpgradeCostFor(CurrentRarity, TargetRarity, Cost)) return;

    // 4. Validate gold
    if (Attr->GetGold() < Cost) return;

    // 5. Deduct gold and update rarity
    Attr->AddGold(-Cost);
    Item->Rarity = TargetRarity;

    // Rarity is replicated on ItemBase, so this will propagate to clients.
}

void UInventoryComponent::RemoveItem(AItemBase* ItemToRemove)
{
    if (!ItemToRemove) return;

    // Deactivate if it was a perk in a slot
    for (int32 i = 0; i < PerkSlots.Num(); i++)
    {
        if (PerkSlots[i] == ItemToRemove)
        {
            if (APerkBase* Perk = Cast<APerkBase>(ItemToRemove)) Perk->OnPerkDeactivated(Cast<APlayerCharacter>(GetOwner()));
            PerkSlots[i] = nullptr;
        }
    }

    ItemToRemove->SetOwner(nullptr);

    for (int32 i = 0; i < MainHandSlots.Num(); i++) if (MainHandSlots[i] == ItemToRemove) MainHandSlots[i] = nullptr;
    for (int32 i = 0; i < OffHandSlots.Num(); i++) if (OffHandSlots[i] == ItemToRemove) OffHandSlots[i] = nullptr;
    for (int32 i = 0; i < InventorySlots.Num(); i++) if (InventorySlots[i] == ItemToRemove) InventorySlots[i] = nullptr;
    for (int32 i = 0; i < AbilitySlots.Num(); i++) if (AbilitySlots[i] == ItemToRemove) AbilitySlots[i] = nullptr;
        
    if (ArmorSet == ItemToRemove) ArmorSet = nullptr;
    if (ConsumableSlot == ItemToRemove) ConsumableSlot = nullptr;
}
