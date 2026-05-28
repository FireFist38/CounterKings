#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ItemBase.h"
#include "AbilityBase.h"
#include "PerkBase.h"
#include "CKGameInstance.generated.h"

/**
 * Game Instance that persists data across ServerTravel.
 * This is necessary because PlayerState objects are destroyed and recreated during map travel.
 */
UCLASS()
class LOWPOLY_API UCKGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UCKGameInstance();

    // --- Match Persistent Data ---
    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedLevel = 1;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    float SavedXP = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedGold = 0;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedAttributePoints = 0;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    float SavedStrength = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    float SavedDexterity = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    float SavedMagic = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    float SavedLuck = 0.0f;

    // --- Match Health (persists across rounds) ---
    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedMatchHealth = 100;

    // --- Shop Pool ---
    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<FName> SavedShopPool;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<bool> SavedLockedSlots;

    // --- Inventory Persistence ---
    // Main Hand Slots (2 slots)
    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<TSubclassOf<AItemBase>> SavedMainHandClasses;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedMainHandRarities;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedMainHandGoldValues;

    // Off Hand Slots (2 slots)
    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<TSubclassOf<AItemBase>> SavedOffHandClasses;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedOffHandRarities;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedOffHandGoldValues;

    // Bag Inventory Slots (12 slots)
    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<TSubclassOf<AItemBase>> SavedInventoryClasses;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedInventoryRarities;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedInventoryGoldValues;

    // Ability Slots (3 slots)
    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<TSubclassOf<AAbilityBase>> SavedAbilityClasses;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedAbilityRarities;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedAbilityGoldValues;

    // Perk Slots (10 slots)
    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<TSubclassOf<APerkBase>> SavedPerkClasses;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedPerkRarities;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedPerkGoldValues;

    // Armor Slot (1 slot)
    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TSubclassOf<AItemBase> SavedArmorClass;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedArmorRarity = 0;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedArmorGoldValue = 0;

    // Consumable Slot (1 slot)
    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    TSubclassOf<AItemBase> SavedConsumableClass;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedConsumableRarity = 0;

    UPROPERTY(BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedConsumableGoldValue = 0;

    // --- Helper Functions ---
    void ClearAllSavedData();

    /** Called once when the game instance starts (not during ServerTravel). Clears stale persistence data. */
    virtual void OnStart() override;

    /** Called when the PIE/game session shuts down. Clears the rooted persistence singleton. */
    virtual void Shutdown() override;
};
