#include "ArmorBase.h"
#include "Net/UnrealNetwork.h"

AArmorBase::AArmorBase()
{
	ItemType = EItemType::Armor;
	HealthBonus = 0.0f;
	MovementSpeedModifier = 1.0f;
}

void AArmorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AArmorBase, HealthBonus);
	DOREPLIFETIME(AArmorBase, MovementSpeedModifier);
}
