#include "TomeBase.h"
#include "AttributeComponent.h"
#include "MagicWeaponBase.h"
#include "PlayerCharacter.h"
#include "SpellBase.h"
#include "SpellChannelBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ATomeBase::ATomeBase()
{
	ItemType = EItemType::Magic;
	bUseCrosshair = true;
	CrosshairStyle = ECrosshairStyle::Offhand;
	ActionName = NSLOCTEXT("Skill", "TomeUse", "Invoke Tome");
}

void ATomeBase::UseOffHand(APlayerCharacter* Character)
{
	if (!HasAuthority() || !Character)
	{
		return;
	}

	if (TomeUseMode == ETomeUseMode::Channel)
	{
		StartChannelInternal(Character);
		return;
	}

	if (!CanActivateNow())
	{
		return;
	}

	float ConsumedMana = 0.0f;
	float ConsumedStamina = 0.0f;
	if (!TryConsumeActivationCost(Character, ConsumedMana, ConsumedStamina))
	{
		return;
	}

	bool bSucceeded = false;
	if (bUseTomeSpellObject && TomeSpell)
	{
		bSucceeded = TryRunConfiguredSpellInstant(Character);
	}
	else
	{
		bSucceeded = BP_OnTomeInstantUse(Character);
	}

	if (!bSucceeded)
	{
		RefundActivationCost(Character, ConsumedMana, ConsumedStamina);
		return;
	}

	LastUseTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	if (InstantCastMontage)
	{
		Character->Multicast_PlayOffhandAttackMontage(InstantCastMontage, NAME_None);
	}

	if (CastSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CastSound, GetActorLocation());
	}
}

void ATomeBase::StopUseOffHand(APlayerCharacter* Character)
{
	if (!HasAuthority() || !Character)
	{
		return;
	}

	if (TomeUseMode == ETomeUseMode::Channel && bIsChanneling)
	{
		StopChannelInternal();
	}
}

FSkillSlotInfo ATomeBase::GetSkillInfo() const
{
	FSkillSlotInfo Info = Super::GetSkillInfo();
	const float CooldownDuration = GetCurrentCooldownDuration();
	Info.bHasCooldown = CooldownDuration > 0.0f;

	if (Info.bHasCooldown && GetWorld())
	{
		const float Now = GetWorld()->GetTimeSeconds();
		Info.CooldownProgress = FMath::Clamp((Now - LastUseTime) / CooldownDuration, 0.0f, 1.0f);
	}
	else
	{
		Info.CooldownProgress = 1.0f;
	}

	return Info;
}

EWeaponHoldStance ATomeBase::GetHoldStance_Implementation() const
{
	return TomeHoldStance;
}

void ATomeBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATomeBase, bIsChanneling);
}

bool ATomeBase::BP_OnTomeInstantUse_Implementation(APlayerCharacter* Character)
{
	return false;
}

bool ATomeBase::BP_OnTomeChannelStart_Implementation(APlayerCharacter* Character)
{
	return true;
}

bool ATomeBase::BP_OnTomeChannelTick_Implementation(APlayerCharacter* Character, float DeltaTime)
{
	return true;
}

void ATomeBase::BP_OnTomeChannelEnd_Implementation(APlayerCharacter* Character)
{
}

bool ATomeBase::CanActivateNow() const
{
	const float CooldownDuration = GetCurrentCooldownDuration();
	if (CooldownDuration <= 0.0f)
	{
		return true;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	return (Now - LastUseTime) >= CooldownDuration;
}

float ATomeBase::GetCurrentCooldownDuration() const
{
	if (bUseSpellCooldown && TomeSpell)
	{
		return FMath::Max(0.0f, TomeSpell->Cooldown);
	}

	return FMath::Max(0.0f, Cooldown);
}

bool ATomeBase::TryConsumeActivationCost(APlayerCharacter* Character, float& OutConsumedMana, float& OutConsumedStamina) const
{
	OutConsumedMana = 0.0f;
	OutConsumedStamina = 0.0f;

	if (!Character)
	{
		return false;
	}

	UAttributeComponent* Attr = Character->GetAttributeComponent();
	if (!Attr)
	{
		return false;
	}

	if (bActivationUsesMana)
	{
		const float ManaCost = bUseTomeSpellObject && TomeSpell ? TomeSpell->ManaCost : ActivationManaCost;
		if (ManaCost > 0.0f && !Attr->ConsumeMana(ManaCost))
		{
			return false;
		}
		OutConsumedMana = ManaCost;
	}
	else
	{
		const float StaminaCost = bUseTomeSpellObject && TomeSpell ? TomeSpell->StaminaCost : ActivationStaminaCost;
		if (StaminaCost > 0.0f && !Attr->ConsumeStamina(StaminaCost))
		{
			return false;
		}
		OutConsumedStamina = StaminaCost;
	}

	return true;
}

void ATomeBase::RefundActivationCost(APlayerCharacter* Character, float ConsumedMana, float ConsumedStamina) const
{
	if (!Character)
	{
		return;
	}

	UAttributeComponent* Attr = Character->GetAttributeComponent();
	if (!Attr)
	{
		return;
	}

	if (ConsumedMana > 0.0f)
	{
		Attr->RestoreMana(ConsumedMana);
	}

	if (ConsumedStamina > 0.0f)
	{
		Attr->RestoreStamina(ConsumedStamina);
	}
}

AMagicWeaponBase* ATomeBase::ResolveSpellSource(APlayerCharacter* Character) const
{
	if (!Character)
	{
		return nullptr;
	}

	AMainHandBase* ActiveMain = Character->GetActiveMainHandWeapon();
	AMagicWeaponBase* Source = Cast<AMagicWeaponBase>(ActiveMain);

	if (bRequireMagicMainHandForSpell)
	{
		return Source;
	}

	return Source;
}

bool ATomeBase::TryRunConfiguredSpellInstant(APlayerCharacter* Character)
{
	if (!TomeSpell || !Character)
	{
		return false;
	}

	AMagicWeaponBase* SpellSource = ResolveSpellSource(Character);
	if (bRequireMagicMainHandForSpell && !SpellSource)
	{
		return false;
	}

	return TomeSpell->Cast(Character, SpellSource);
}

bool ATomeBase::StartChannelInternal(APlayerCharacter* Character)
{
	if (bIsChanneling || !Character || !CanActivateNow())
	{
		return false;
	}

	float ConsumedMana = 0.0f;
	float ConsumedStamina = 0.0f;
	if (!TryConsumeActivationCost(Character, ConsumedMana, ConsumedStamina))
	{
		return false;
	}

	USpellChannelBase* ChannelSpell = bUseTomeSpellObject ? Cast<USpellChannelBase>(TomeSpell) : nullptr;
	AMagicWeaponBase* SpellSource = nullptr;
	bool bStarted = false;

	if (ChannelSpell)
	{
		SpellSource = ResolveSpellSource(Character);
		if (!bRequireMagicMainHandForSpell || SpellSource)
		{
			bStarted = ChannelSpell->StartChannel(Character, SpellSource);
		}
	}
	else
	{
		bStarted = BP_OnTomeChannelStart(Character);
	}

	if (!bStarted)
	{
		RefundActivationCost(Character, ConsumedMana, ConsumedStamina);
		return false;
	}

	bIsChanneling = true;
	LastUseTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	ActiveChannelCaster = Character;
	ActiveSpellSource = SpellSource;

	if (ChannelMontage)
	{
		Character->Multicast_PlayOffhandAttackMontage(ChannelMontage, NAME_None);
	}

	if (ChannelStartSound)
	{
		ChannelAudio = UGameplayStatics::SpawnSoundAtLocation(this, ChannelStartSound, GetActorLocation());
	}

	HandleChannelTick();

	if (bIsChanneling)
	{
		const float TickInterval = FMath::Max(ChannelTickRate, 0.01f);
		GetWorldTimerManager().SetTimer(ChannelTimerHandle, this, &ATomeBase::HandleChannelTick, TickInterval, true);
	}

	return true;
}

void ATomeBase::StopChannelInternal()
{
	GetWorldTimerManager().ClearTimer(ChannelTimerHandle);

	if (!bIsChanneling)
	{
		return;
	}

	APlayerCharacter* Caster = ActiveChannelCaster;
	USpellChannelBase* ChannelSpell = bUseTomeSpellObject ? Cast<USpellChannelBase>(TomeSpell) : nullptr;

	if (ChannelSpell)
	{
		ChannelSpell->EndChannel(Caster, ActiveSpellSource);
	}
	else if (Caster)
	{
		BP_OnTomeChannelEnd(Caster);
	}

	if (Caster && ChannelMontage)
	{
		Caster->Multicast_StopMontage(ChannelMontage, 0.15f);
	}

	// Stop the channel start sound if it's still playing
	if (ChannelAudio && ChannelAudio->IsPlaying())
	{
		ChannelAudio->FadeOut(0.15f, 0.0f);
	}
	ChannelAudio = nullptr;

	bIsChanneling = false;
	ActiveChannelCaster = nullptr;
	ActiveSpellSource = nullptr;
}

void ATomeBase::HandleChannelTick()
{
	if (!bIsChanneling)
	{
		StopChannelInternal();
		return;
	}

	APlayerCharacter* Caster = ActiveChannelCaster;
	if (!Caster)
	{
		StopChannelInternal();
		return;
	}

	const float TickDelta = FMath::Max(ChannelTickRate, 0.01f);
	bool bContinue = true;

	USpellChannelBase* ChannelSpell = bUseTomeSpellObject ? Cast<USpellChannelBase>(TomeSpell) : nullptr;
	if (ChannelSpell)
	{
		// Removed the strict ActiveSpellSource requirement here. 
		// This allows tomes to channel spells even if the player is holding a sword.
		bContinue = ChannelSpell->TickChannel(Caster, ActiveSpellSource, TickDelta);
	}
	else
	{
		UAttributeComponent* Attr = Caster->GetAttributeComponent();
		if (!Attr)
		{
			bContinue = false;
		}
		else
		{
			const float DrainAmount = ChannelDrainPerSecond * TickDelta;
			const bool bPaidDrain = bChannelDrainUsesMana ? Attr->ConsumeMana(DrainAmount) : Attr->ConsumeStamina(DrainAmount);
			bContinue = bPaidDrain && BP_OnTomeChannelTick(Caster, TickDelta);
		}
	}

	if (!bContinue)
	{
		StopChannelInternal();
	}
}
