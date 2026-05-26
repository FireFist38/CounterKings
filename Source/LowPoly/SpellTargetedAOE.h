#pragma once

#include "CoreMinimal.h"
#include "SpellBase.h"
#include "ItemBase.h"
#include "SpellTargetedAOE.generated.h"

class ATargetedAOEIndicator;
class ATargetedAOEDetonation;

/**
 * Two-phase targeted AOE spell.
 * First cast: projects a targeting circle onto the ground under the caster's crosshair.
 * Second cast: confirms the location, destroys the indicator, and detonates the AOE effect.
 */
UCLASS(Blueprintable, EditInlineNew)
class LOWPOLY_API USpellTargetedAOE : public USpellBase
{
	GENERATED_BODY()

public:
	USpellTargetedAOE();

	virtual bool Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|TargetedAOE")
	TSubclassOf<ATargetedAOEIndicator> IndicatorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|TargetedAOE")
	TSubclassOf<ATargetedAOEDetonation> DetonationClass;

	/** Damage applied at the confirmed target location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|TargetedAOE")
	FDamageBundle DetonationDamage;

	/** Radius of both the indicator ring and the detonation damage sphere */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|TargetedAOE")
	float TargetRadius = 400.0f;

	/** How far from the caster the ground can be targeted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|TargetedAOE")
	float MaxTargetRange = 2000.0f;

	/** How long the detonation effect lingers before destroying */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|TargetedAOE")
	float DetonationLifetime = 3.0f;

private:
	/** True while the indicator is active and waiting for a confirm cast */
	bool bIsTargeting = false;

	UPROPERTY()
	ATargetedAOEIndicator* ActiveIndicator = nullptr;
};
