#include "MagicWeaponBase.h"
#include "SpellBase.h"
#include "SpellChannelBase.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "Kismet/GameplayStatics.h"

AMagicWeaponBase::AMagicWeaponBase()
{
	ItemType = EItemType::Magic;
	bTwoHanded = true;
	MagicScaling = 1.0f;
	StrengthScaling = 0.0f;
    bUseCrosshair = true;
}

bool AMagicWeaponBase::IsMagic() const
{
	return true;
}

bool AMagicWeaponBase::CanCastPrimary() const
{
	if (!PrimarySpell) return false;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	return (Now - LastPrimaryCastTime) >= PrimarySpell->Cooldown;
}

bool AMagicWeaponBase::CanCastSecondary() const
{
	if (!SecondarySpell) return false;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	return (Now - LastSecondaryCastTime) >= SecondarySpell->Cooldown;
}

FName AMagicWeaponBase::GetCastSocketName() const
{
	return CastSocketName;
}

USpellBase* AMagicWeaponBase::GetPrimarySpell() const
{
	return PrimarySpell;
}

USpellBase* AMagicWeaponBase::GetSecondarySpell() const
{
	return SecondarySpell;
}

bool AMagicWeaponBase::CastPrimary(APlayerCharacter* Caster)
{
	if (!CanCastPrimary()) return false;
	return TryCast(PrimarySpell, LastPrimaryCastTime, Caster, PrimarySpellMontage, true);
}

bool AMagicWeaponBase::CastSecondary(APlayerCharacter* Caster)
{
	if (!CanCastSecondary()) return false;
	return TryCast(SecondarySpell, LastSecondaryCastTime, Caster, SecondarySpellMontage, false);
}

void AMagicWeaponBase::StartPrimaryChannel(APlayerCharacter* Caster)
{
    if (!HasAuthority() || !CanCastPrimary() || !Caster) return;
    
    if (USpellChannelBase* ChannelSpell = Cast<USpellChannelBase>(PrimarySpell))
    {
        UAttributeComponent* Attr = Caster->GetAttributeComponent();
        if (!Attr) return;

        const bool bPaidCost = ChannelSpell->bUseMana
            ? Attr->ConsumeMana(ChannelSpell->ManaCost)
            : Attr->ConsumeStamina(ChannelSpell->StaminaCost);
        if (!bPaidCost) return;

        if (ChannelSpell->StartChannel(Caster, this))
        {
            LastPrimaryCastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            if (PrimarySpellMontage)
                Caster->Multicast_PlayAttackMontage(PrimarySpellMontage, NAME_None);
            Multicast_OnSpellCast(true);

            if (!ChannelTick(Caster, true))
            {
                StopPrimaryChannel();
                return;
            }
            const float SafeTickRate = FMath::Max(ChannelSpell->TickRate, 0.01f);
            GetWorldTimerManager().SetTimer(PrimaryChannelTimerHandle, [this, Caster]()
            {
                if (!ChannelTick(Caster, true))
                {
                    StopPrimaryChannel();
                }
            }, SafeTickRate, true);
        }
        else
        {
            if (ChannelSpell->bUseMana)
                Attr->RestoreMana(ChannelSpell->ManaCost);
            else
                Attr->RestoreStamina(ChannelSpell->StaminaCost);
        }
    }
}

void AMagicWeaponBase::StopPrimaryChannel()
{
    GetWorldTimerManager().ClearTimer(PrimaryChannelTimerHandle);

    APlayerCharacter* Caster = Cast<APlayerCharacter>(GetOwner());
    if (!Caster)
    {
        Caster = Cast<APlayerCharacter>(GetAttachParentActor());
    }

    if (USpellChannelBase* ChannelSpell = Cast<USpellChannelBase>(PrimarySpell))
    {
        ChannelSpell->EndChannel(Caster, this);

        if (Caster && PrimarySpellMontage)
        {
            Caster->Multicast_StopMontage(PrimarySpellMontage, 0.15f);
        }

        if (Caster)
        {
            Caster->SetRotationClamp(false, 0.0f);
        }
    }
}

void AMagicWeaponBase::StartSecondaryChannel(APlayerCharacter* Caster)
{
    if (!HasAuthority() || !CanCastSecondary() || !Caster) return;
    
    if (USpellChannelBase* ChannelSpell = Cast<USpellChannelBase>(SecondarySpell))
    {
        UAttributeComponent* Attr = Caster->GetAttributeComponent();
        if (!Attr) return;

        const bool bPaidCost = ChannelSpell->bUseMana
            ? Attr->ConsumeMana(ChannelSpell->ManaCost)
            : Attr->ConsumeStamina(ChannelSpell->StaminaCost);
        if (!bPaidCost) return;

        if (ChannelSpell->StartChannel(Caster, this))
        {
            LastSecondaryCastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            if (SecondarySpellMontage)
                Caster->Multicast_PlayAttackMontage(SecondarySpellMontage, NAME_None);
            Multicast_OnSpellCast(false);

            if (!ChannelTick(Caster, false))
            {
                StopSecondaryChannel();
                return;
            }
            const float SafeTickRate = FMath::Max(ChannelSpell->TickRate, 0.01f);
            GetWorldTimerManager().SetTimer(SecondaryChannelTimerHandle, [this, Caster]()
            {
                if (!ChannelTick(Caster, false))
                {
                    StopSecondaryChannel();
                }
            }, SafeTickRate, true);
        }
        else
        {
            if (ChannelSpell->bUseMana)
                Attr->RestoreMana(ChannelSpell->ManaCost);
            else
                Attr->RestoreStamina(ChannelSpell->StaminaCost);
        }
    }
}

void AMagicWeaponBase::StopSecondaryChannel()
{
    GetWorldTimerManager().ClearTimer(SecondaryChannelTimerHandle);

    APlayerCharacter* Caster = Cast<APlayerCharacter>(GetOwner());
    if (!Caster)
    {
        Caster = Cast<APlayerCharacter>(GetAttachParentActor());
    }

    if (USpellChannelBase* ChannelSpell = Cast<USpellChannelBase>(SecondarySpell))
    {
        ChannelSpell->EndChannel(Caster, this);

        if (Caster && SecondarySpellMontage)
        {
            Caster->Multicast_StopMontage(SecondarySpellMontage, 0.15f);
        }

        if (Caster)
        {
            Caster->SetRotationClamp(false, 0.0f);
        }
    }
}

bool AMagicWeaponBase::ChannelTick(APlayerCharacter* Caster, bool bIsPrimary)
{
    USpellChannelBase* ChannelSpell = Cast<USpellChannelBase>(bIsPrimary ? PrimarySpell : SecondarySpell);
    if (!Caster || !ChannelSpell) return false;

    return ChannelSpell->TickChannel(Caster, this, ChannelSpell->TickRate);
}

bool AMagicWeaponBase::TryCast(USpellBase* Spell, float& LastCastTimeRef, APlayerCharacter* Caster, UAnimMontage* SpellMontage, bool bIsPrimary)
{
	if (!HasAuthority() || !Spell || !Caster) return false;

	UAttributeComponent* Attr = Caster->GetAttributeComponent();
	if (!Attr) return false;

	if (Spell->bUseMana)
	{
		if (!Attr->ConsumeMana(Spell->ManaCost)) return false;
	}
	else
	{
		if (!Attr->ConsumeStamina(Spell->StaminaCost)) return false;
	}

	LastCastTimeRef = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	if (SpellMontage)
		Caster->Multicast_PlayAttackMontage(SpellMontage, NAME_None);
	Multicast_OnSpellCast(bIsPrimary);

	if (Spell->CastTime > 0.0f)
	{
		TWeakObjectPtr<USpellBase> WeakSpell = Spell;
		TWeakObjectPtr<APlayerCharacter> WeakCaster = Caster;
		TWeakObjectPtr<AMagicWeaponBase> WeakStaff = this;

		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, [WeakSpell, WeakCaster, WeakStaff]()
		{
			if (WeakSpell.IsValid() && WeakCaster.IsValid() && WeakStaff.IsValid())
			{
				if (WeakCaster->IsValidLowLevel() && WeakCaster->GetAttributeComponent() && WeakCaster->GetAttributeComponent()->IsDead()) return;
				WeakSpell->Cast(WeakCaster.Get(), WeakStaff.Get());
			}
		}, Spell->CastTime, false);
		return true;
	}

	const bool bCast = Spell->Cast(Caster, this);
	if (!bCast)
	{
		if (Spell->bUseMana)
			Attr->RestoreMana(Spell->ManaCost);
		else
			Attr->RestoreStamina(Spell->StaminaCost);
	}
	return bCast;
}

EWeaponHoldStance AMagicWeaponBase::GetHoldStance_Implementation() const
{
	return EWeaponHoldStance::Empty;
}

static FSkillSlotInfo BuildSpellSlot(const USpellBase* Spell, float LastCastTime, UWorld* World)
{
	FSkillSlotInfo Info;
	if (!Spell) return Info;

	Info.bIsValid = true;
	Info.Icon = Spell->Icon;
	Info.DisplayName = FText::FromName(Spell->DisplayName);
	Info.bHasCooldown = Spell->Cooldown > 0.0f;

	if (Info.bHasCooldown && World)
	{
		const float Now = World->GetTimeSeconds();
		Info.CooldownProgress = FMath::Clamp((Now - LastCastTime) / Spell->Cooldown, 0.0f, 1.0f);
	}
	else
	{
		Info.CooldownProgress = 1.0f;
	}
	return Info;
}

FSkillSlotInfo AMagicWeaponBase::GetLMBSkillInfo() const
{
	return BuildSpellSlot(PrimarySpell, LastPrimaryCastTime, GetWorld());
}

FSkillSlotInfo AMagicWeaponBase::GetRMBSkillInfo() const
{
	return BuildSpellSlot(SecondarySpell, LastSecondaryCastTime, GetWorld());
}

void AMagicWeaponBase::Multicast_OnSpellCast_Implementation(bool bIsPrimary)
{
    USoundBase* SoundToPlay = bIsPrimary ? PrimaryCastSound : SecondaryCastSound;
	if (SoundToPlay)
		UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, GetActorLocation());
	BP_OnSpellCast(bIsPrimary);
}
