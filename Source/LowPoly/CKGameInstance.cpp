#include "CKGameInstance.h"
#include "CKPersistenceManager.h"

UCKGameInstance::UCKGameInstance()
{
    SavedMatchHealth = 100;
}

void UCKGameInstance::OnStart()
{
    Super::OnStart();

    // Clear the PersistenceManager singleton on new PIE session start.
    // OnStart() is called once per game session (including PIE start).
    // It is NOT called during ServerTravel transitions, so data survives
    // map travel within the same PIE session but gets cleared when PIE stops/restarts.
    UCKPersistenceManager::Get().ClearAllSavedData();
}

void UCKGameInstance::Shutdown()
{
    UCKPersistenceManager::Get().ClearAllSavedData();

    Super::Shutdown();
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
