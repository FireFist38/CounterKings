#include "OffHandBase.h"
#include "Net/UnrealNetwork.h"

AOffHandBase::AOffHandBase()
{
}

void AOffHandBase::UseOffHand(class APlayerCharacter* Character)
{
}

void AOffHandBase::StopUseOffHand(class APlayerCharacter* Character)
{
}

FSkillSlotInfo AOffHandBase::GetSkillInfo() const
{
	FSkillSlotInfo Info;
	Info.bIsValid = true;
	Info.Icon = ActionIcon;
	Info.DisplayName = ActionName;
	Info.bHasCooldown = false;
	Info.CooldownProgress = 1.0f;
	return Info;
}

EWeaponHoldStance AOffHandBase::GetHoldStance_Implementation() const
{
	// Default: no stance — shields/bucklers ride in idle. Override in BP for tomes/throwables/etc.
	return EWeaponHoldStance::Empty;
}

void AOffHandBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}