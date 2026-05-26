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
}
