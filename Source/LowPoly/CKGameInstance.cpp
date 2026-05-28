#include "CKGameInstance.h"
#include "CKPersistenceManager.h"

UCKGameInstance::UCKGameInstance()
{
    SavedMatchHealth = 100;

    // Clear PersistenceManager singleton on GameInstance init
    // This ensures PIE sessions start fresh
    UCKPersistenceManager::Get().ClearAllSavedData();
}

void UCKGameInstance::ClearAllSavedData()
{
    SavedLevel = 1;
    SavedXP = 0.0f;
    SavedGold = 0;
    SavedAttributePoints = 0;
    SavedStrength = 0.0f;
    SavedDexterity = 0.0f;
    SavedMagic = 0.0f;
    SavedLuck = 0.0f;
    SavedMatchHealth = 100;

    SavedShopPool.Empty();
    SavedLockedSlots.Empty();

    SavedMainHandClasses.Empty();
    SavedMainHandRarities.Empty();
    SavedMainHandGoldValues.Empty();

    SavedOffHandClasses.Empty();
    SavedOffHandRarities.Empty();
    SavedOffHandGoldValues.Empty();

    SavedInventoryClasses.Empty();
    SavedInventoryRarities.Empty();
    SavedInventoryGoldValues.Empty();

    SavedAbilityClasses.Empty();
    SavedAbilityRarities.Empty();
    SavedAbilityGoldValues.Empty();

    SavedPerkClasses.Empty();
    SavedPerkRarities.Empty();
    SavedPerkGoldValues.Empty();

    SavedArmorClass = nullptr;
    SavedArmorRarity = 0;
    SavedArmorGoldValue = 0;

    SavedConsumableClass = nullptr;
    SavedConsumableRarity = 0;
    SavedConsumableGoldValue = 0;
}
