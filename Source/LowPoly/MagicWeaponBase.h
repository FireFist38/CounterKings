#pragma once

#include "CoreMinimal.h"
#include "MainHandBase.h"
#include "MagicWeaponBase.generated.h"

class USpellBase;
class USpellChannelBase;
class APlayerCharacter;

UCLASS(Abstract)
class LOWPOLY_API AMagicWeaponBase : public AMainHandBase
{
	GENERATED_BODY()

public:
	AMagicWeaponBase();

	virtual bool IsMagic() const override;

	virtual FSkillSlotInfo GetLMBSkillInfo() const override;
	virtual FSkillSlotInfo GetRMBSkillInfo() const override;

	virtual EWeaponHoldStance GetHoldStance_Implementation() const override;

	bool CastPrimary(APlayerCharacter* Caster);
	bool CastSecondary(APlayerCharacter* Caster);

    void StartPrimaryChannel(APlayerCharacter* Caster);
    void StopPrimaryChannel();
    void StartSecondaryChannel(APlayerCharacter* Caster);
    void StopSecondaryChannel();

	UFUNCTION(BlueprintCallable, Category = "CK|Magic")
	bool CanCastPrimary() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Magic")
	bool CanCastSecondary() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Magic")
	FName GetCastSocketName() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Magic")
	USpellBase* GetPrimarySpell() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Magic")
	USpellBase* GetSecondarySpell() const;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnSpellCast(bool bIsPrimary);

	UFUNCTION(BlueprintImplementableEvent, Category = "CK|Magic")
	void BP_OnSpellCast(bool bIsPrimary);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "CK|Magic")
	USpellBase* PrimarySpell;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "CK|Magic")
	USpellBase* SecondarySpell;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Magic")
	FName CastSocketName = FName("CastSocket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Magic")
	UAnimMontage* PrimarySpellMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Magic")
	UAnimMontage* SecondarySpellMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Magic")
	class USoundBase* PrimaryCastSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Magic")
	class USoundBase* SecondaryCastSound;

private:
	float LastPrimaryCastTime = -999.0f;
	float LastSecondaryCastTime = -999.0f;
    FTimerHandle PrimaryChannelTimerHandle;
    FTimerHandle SecondaryChannelTimerHandle;

	bool TryCast(USpellBase* Spell, float& LastCastTimeRef, APlayerCharacter* Caster, UAnimMontage* SpellMontage, bool bIsPrimary);
    bool ChannelTick(APlayerCharacter* Caster, bool bIsPrimary);
};
