#pragma once

#include "CoreMinimal.h"
#include "OffHandBase.h"
#include "WeaponStanceTypes.h"
#include "Components/AudioComponent.h"
#include "TomeBase.generated.h"

class APlayerCharacter;
class AMagicWeaponBase;
class UAnimMontage;
class USoundBase;
class USpellBase;

UENUM(BlueprintType)
enum class ETomeUseMode : uint8
{
	Instant,
	Channel,
	Utility
};

UCLASS(Abstract)
class LOWPOLY_API ATomeBase : public AOffHandBase
{
	GENERATED_BODY()

public:
	ATomeBase();

	virtual void UseOffHand(APlayerCharacter* Character) override;
	virtual void StopUseOffHand(APlayerCharacter* Character) override;
	virtual FSkillSlotInfo GetSkillInfo() const override;
	virtual EWeaponHoldStance GetHoldStance_Implementation() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "CK|Tome")
	bool IsChanneling() const { return bIsChanneling; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome")
	ETomeUseMode TomeUseMode = ETomeUseMode::Instant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "CK|Tome|Spell")
	USpellBase* TomeSpell = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Spell")
	bool bUseTomeSpellObject = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Spell")
	bool bRequireMagicMainHandForSpell = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float Cooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Timing")
	bool bUseSpellCooldown = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Cost")
	bool bActivationUsesMana = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Cost", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ActivationManaCost = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Cost", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ActivationStaminaCost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Channel", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float ChannelTickRate = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Channel")
	bool bChannelDrainUsesMana = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Channel", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ChannelDrainPerSecond = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Animation")
	UAnimMontage* InstantCastMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Animation")
	UAnimMontage* ChannelMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Audio")
	USoundBase* CastSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Audio")
	USoundBase* ChannelStartSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tome|Stance")
	EWeaponHoldStance TomeHoldStance = EWeaponHoldStance::OneHandedMelee;

	UFUNCTION(BlueprintNativeEvent, Category = "CK|Tome")
	bool BP_OnTomeInstantUse(APlayerCharacter* Character);
	virtual bool BP_OnTomeInstantUse_Implementation(APlayerCharacter* Character);

	UFUNCTION(BlueprintNativeEvent, Category = "CK|Tome")
	bool BP_OnTomeChannelStart(APlayerCharacter* Character);
	virtual bool BP_OnTomeChannelStart_Implementation(APlayerCharacter* Character);

	UFUNCTION(BlueprintNativeEvent, Category = "CK|Tome")
	bool BP_OnTomeChannelTick(APlayerCharacter* Character, float DeltaTime);
	virtual bool BP_OnTomeChannelTick_Implementation(APlayerCharacter* Character, float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, Category = "CK|Tome")
	void BP_OnTomeChannelEnd(APlayerCharacter* Character);
	virtual void BP_OnTomeChannelEnd_Implementation(APlayerCharacter* Character);

private:
	UPROPERTY(Replicated)
	bool bIsChanneling = false;

	/** Audio component for the channel start sound, so we can stop it on channel end */
	UPROPERTY()
	UAudioComponent* ChannelAudio = nullptr;

	float LastUseTime = -999.0f;
	FTimerHandle ChannelTimerHandle;

	UPROPERTY()
	APlayerCharacter* ActiveChannelCaster = nullptr;

	UPROPERTY()
	AMagicWeaponBase* ActiveSpellSource = nullptr;

	bool CanActivateNow() const;
	float GetCurrentCooldownDuration() const;
	bool TryConsumeActivationCost(APlayerCharacter* Character, float& OutConsumedMana, float& OutConsumedStamina) const;
	void RefundActivationCost(APlayerCharacter* Character, float ConsumedMana, float ConsumedStamina) const;
	AMagicWeaponBase* ResolveSpellSource(APlayerCharacter* Character) const;
	bool TryRunConfiguredSpellInstant(APlayerCharacter* Character);
	bool StartChannelInternal(APlayerCharacter* Character);
	void StopChannelInternal();
	void HandleChannelTick();
};
