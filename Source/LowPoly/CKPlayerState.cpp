#include "CKPlayerState.h"
#include "Net/UnrealNetwork.h"

ACKPlayerState::ACKPlayerState()
{
    MatchHealth = 100;
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
    DOREPLIFETIME(ACKPlayerState, SavedHealth);
    DOREPLIFETIME(ACKPlayerState, SavedInventory);
    DOREPLIFETIME(ACKPlayerState, SavedShopPool);
    DOREPLIFETIME(ACKPlayerState, SavedLockedSlots);
}