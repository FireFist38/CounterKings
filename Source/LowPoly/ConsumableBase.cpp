#include "ConsumableBase.h"
#include "Net/UnrealNetwork.h"

AConsumableBase::AConsumableBase()
{
	UsesRemaining = 1;
}

void AConsumableBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AConsumableBase, UsesRemaining);
}
