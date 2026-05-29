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

float AArmorBase::GetHealthBonusForRarity(EItemRarity TargetRarity) const
{
    FRarityStats Stats;
    if (GetRarityStatsFor(TargetRarity, Stats)) return Stats.HealthBonus;
    return HealthBonus;
}

float AArmorBase::GetMovementSpeedModifierForRarity(EItemRarity TargetRarity) const
{
    FRarityStats Stats;
    if (GetRarityStatsFor(TargetRarity, Stats)) return Stats.MovementSpeedModifier;
    return MovementSpeedModifier;
}

