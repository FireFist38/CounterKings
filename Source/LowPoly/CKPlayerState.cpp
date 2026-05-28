#include "CKPlayerState.h"
#include "Net/UnrealNetwork.h"

ACKPlayerState::ACKPlayerState()
{
    MatchHealth = 100;
    SavedMatchHealth = 100;
}

void ACKPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACKPlayerState, MatchHealth);

    DOREPLIFETIME(ACKPlayerState, SavedLevel);
    DOREPLIFETIME(ACKPlayerState, SavedXP);
    DOREPLIFETIME(ACKPlayerState, SavedGold);
    DOREPLIFETIME(ACKPlayerState, SavedAttributePoints);
    DOREPLIFETIME(ACKPlayerState, SavedStrength);
    DOREPLIFETIME(ACKPlayerState, SavedDexterity);
    DOREPLIFETIME(ACKPlayerState, SavedMagic);
    DOREPLIFETIME(ACKPlayerState, SavedLuck);

    DOREPLIFETIME(ACKPlayerState, SavedShopPool);
    DOREPLIFETIME(ACKPlayerState, SavedLockedSlots);

    // Inventory Persistence
    DOREPLIFETIME(ACKPlayerState, SavedMainHandClasses);
    DOREPLIFETIME(ACKPlayerState, SavedMainHandRarities);
    DOREPLIFETIME(ACKPlayerState, SavedMainHandGoldValues);

    DOREPLIFETIME(ACKPlayerState, SavedOffHandClasses);
    DOREPLIFETIME(ACKPlayerState, SavedOffHandRarities);
    DOREPLIFETIME(ACKPlayerState, SavedOffHandGoldValues);

    DOREPLIFETIME(ACKPlayerState, SavedInventoryClasses);
    DOREPLIFETIME(ACKPlayerState, SavedInventoryRarities);
    DOREPLIFETIME(ACKPlayerState, SavedInventoryGoldValues);

    DOREPLIFETIME(ACKPlayerState, SavedAbilityClasses);
    DOREPLIFETIME(ACKPlayerState, SavedAbilityRarities);
    DOREPLIFETIME(ACKPlayerState, SavedAbilityGoldValues);

    DOREPLIFETIME(ACKPlayerState, SavedPerkClasses);
    DOREPLIFETIME(ACKPlayerState, SavedPerkRarities);
    DOREPLIFETIME(ACKPlayerState, SavedPerkGoldValues);

    DOREPLIFETIME(ACKPlayerState, SavedArmorClass);
    DOREPLIFETIME(ACKPlayerState, SavedArmorRarity);
    DOREPLIFETIME(ACKPlayerState, SavedArmorGoldValue);

    DOREPLIFETIME(ACKPlayerState, SavedConsumableClass);
    DOREPLIFETIME(ACKPlayerState, SavedConsumableRarity);
    DOREPLIFETIME(ACKPlayerState, SavedConsumableGoldValue);

    DOREPLIFETIME(ACKPlayerState, SavedMatchHealth);
}