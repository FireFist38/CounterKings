#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "AbilityBase.h"
#include "PerkBase.h"
#include "CKPersistenceManager.generated.h"

/**
 * Singleton manager for persisting player data across ServerTravel.
 * This is necessary because PlayerState and GameState can be reset during map travel.
 */
UCLASS()
class LOWPOLY_API UCKPersistenceManager : public UObject
{
    GENERATED_BODY()

public:
    static UCKPersistenceManager& Get();

    // --- Attributes ---
    int32 SavedLevel = 1;
    float SavedXP = 0.0f;
    int32 SavedGold = 0;
    int32 SavedAttributePoints = 0;
    float SavedStrength = 0.0f;
    float SavedDexterity = 0.0f;
    float SavedMagic = 0.0f;
    float SavedLuck = 0.0f;
    int32 SavedMatchHealth = 100;

    // --- Shop Pool ---
    TArray<FName> SavedShopPool;
    TArray<bool> SavedLockedSlots;

    // --- Inventory Persistence ---
    TArray<TSubclassOf<AItemBase>> SavedMainHandClasses;
    TArray<int32> SavedMainHandRarities;
    TArray<int32> SavedMainHandGoldValues;

    TArray<TSubclassOf<AItemBase>> SavedOffHandClasses;
    TArray<int32> SavedOffHandRarities;
    TArray<int32> SavedOffHandGoldValues;

    TArray<TSubclassOf<AItemBase>> SavedInventoryClasses;
    TArray<int32> SavedInventoryRarities;
    TArray<int32> SavedInventoryGoldValues;

    TArray<TSubclassOf<AAbilityBase>> SavedAbilityClasses;
    TArray<int32> SavedAbilityRarities;
    TArray<int32> SavedAbilityGoldValues;

    TArray<TSubclassOf<APerkBase>> SavedPerkClasses;
    TArray<int32> SavedPerkRarities;
    TArray<int32> SavedPerkGoldValues;

    TSubclassOf<AItemBase> SavedArmorClass;
    int32 SavedArmorRarity = 0;
    int32 SavedArmorGoldValue = 0;

    TSubclassOf<AItemBase> SavedConsumableClass;
    int32 SavedConsumableRarity = 0;
    int32 SavedConsumableGoldValue = 0;

    void ClearAllSavedData();

private:
    UCKPersistenceManager();
    static UCKPersistenceManager* Instance;
};
