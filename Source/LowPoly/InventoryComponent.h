#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemBase.h"
#include "InventoryUpgradeComponent.h"
#include "EquipmentUpgradeTypes.h"
#include "InventoryComponent.generated.h"


class AAbilityBase;
class APerkBase;

// Enum to specify slot type for RPCs
UENUM(BlueprintType)
enum class ESlotGroup : uint8
{
    MainHand,
    OffHand,
    Armor,
    Consumable,
    Bag,
    Ability,
    Perk
};

UENUM(BlueprintType)
enum class EContextType : uint8
{
    Purchase,
    Sell,
    Equip,
    Drop
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LOWPOLY_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()


public:	
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CK|Inventory")
	TArray<AItemBase*> MainHandSlots;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CK|Inventory")
	TArray<AItemBase*> OffHandSlots;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CK|Inventory")
	AItemBase* ArmorSet;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CK|Inventory")
	AItemBase* ConsumableSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Inventory")
	int32 MaxPickupCap;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CK|Inventory")
	int32 ActiveMainHandIndex;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CK|Inventory")
	int32 ActiveOffHandIndex;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CK|Inventory")
	TArray<AItemBase*> InventorySlots;

    // --- Ability System (GDD Section 5.3) ---
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CK|Inventory")
    TArray<AAbilityBase*> AbilitySlots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Inventory|Unlock")
    int32 Ability1LevelThreshold = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Inventory|Unlock")
    int32 Ability2LevelThreshold = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Inventory|Unlock")
    int32 Ability3LevelThreshold = 9;

    // --- Perk System (GDD Section 5.4) ---
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CK|Inventory")
    TArray<APerkBase*> PerkSlots;

public:
    UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
    TArray<AItemBase*> GetAllInventoryItems() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	TArray<AItemBase*> GetMainHandSlots() const { return MainHandSlots; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	TArray<AItemBase*> GetOffHandSlots() const { return OffHandSlots; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	AItemBase* GetBagItem(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	AItemBase* GetActiveMainHandItem() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	bool IsOffHandLocked() const;

	// Attach or detach the active offhand based on whether the main hand is two-handed.
	void RefreshOffHandAttachment();

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	AItemBase* GetActiveOffHandItem() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	int32 GetActiveMainHandIndex() const { return ActiveMainHandIndex; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	int32 GetActiveOffHandIndex() const { return ActiveOffHandIndex; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	AItemBase* GetConsumableSlot() const { return ConsumableSlot; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	AItemBase* GetArmorSet() const { return ArmorSet; }

    UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
    TArray<AAbilityBase*> GetAbilitySlots() const { return AbilitySlots; }

    UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
    TArray<APerkBase*> GetPerkSlots() const { return PerkSlots; }

    UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
    bool IsAbilitySlotUnlocked(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
    int32 GetLevelThresholdForAbilitySlot(int32 SlotIndex) const;

	// Public accessors for inventory arrays (for persistence)
	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	TArray<AItemBase*> GetMainHandSlotsArray() const { return MainHandSlots; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	TArray<AItemBase*> GetOffHandSlotsArray() const { return OffHandSlots; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	TArray<AItemBase*> GetInventorySlotsArray() const { return InventorySlots; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	TArray<AAbilityBase*> GetAbilitySlotsArray() const { return AbilitySlots; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	TArray<APerkBase*> GetPerkSlotsArray() const { return PerkSlots; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	AItemBase* GetArmorSetPublic() const { return ArmorSet; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	AItemBase* GetConsumableSlotPublic() const { return ConsumableSlot; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	void SetArmorSet(AItemBase* NewArmor) { ArmorSet = NewArmor; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	void SetConsumableSlot(AItemBase* NewConsumable) { ConsumableSlot = NewConsumable; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	void SetMainHandSlot(int32 Index, AItemBase* Item) { if (MainHandSlots.IsValidIndex(Index)) MainHandSlots[Index] = Item; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	void SetOffHandSlot(int32 Index, AItemBase* Item) { if (OffHandSlots.IsValidIndex(Index)) OffHandSlots[Index] = Item; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	void SetInventorySlot(int32 Index, AItemBase* Item) { if (InventorySlots.IsValidIndex(Index)) InventorySlots[Index] = Item; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	void SetAbilitySlot(int32 Index, AAbilityBase* Item) { if (AbilitySlots.IsValidIndex(Index)) AbilitySlots[Index] = Item; }

	UFUNCTION(BlueprintCallable, Category = "CK|Inventory")
	void SetPerkSlot(int32 Index, APerkBase* Item) { if (PerkSlots.IsValidIndex(Index)) PerkSlots[Index] = Item; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Inventory")
	void Server_TryPickupItem(AItemBase* Item);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Inventory")
	void Server_CycleMainHand();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Inventory")
	void Server_CycleOffHand();

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Inventory")
    void Server_SwapItem(AItemBase* NewItem, int32 SlotIndex);

    // New RPC for Drag and Drop
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Inventory")
    void Server_MoveItem(ESlotGroup FromGroup, int32 FromIndex, ESlotGroup ToGroup, int32 ToIndex);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Inventory")
	void Server_DropItem(ESlotGroup FromGroup, int32 FromIndex);

    UFUNCTION(Server, Reliable)
    void Server_EquipItem(AItemBase* Item, ESlotGroup Slot);

    UFUNCTION(Server, Reliable)
    void Server_UnequipItem(ESlotGroup Slot, int32 Index);

    // --- Equipment Upgrade (Rarity) ---
    UFUNCTION(Server, Reliable, BlueprintCallable, Category="CK|Upgrade")
    void Server_RequestUpgradeRarity(AItemBase* Item, EItemUpgradeTarget UpgradeTarget);

    // Server implementation
    void Server_RequestUpgradeRarity_Implementation(AItemBase* Item, EItemUpgradeTarget UpgradeTarget);

    void RemoveItem(AItemBase* ItemToRemove);
};
