#pragma once

#include "CoreMinimal.h"
#include "SpellBase.h"
#include "ItemBase.h"
#include "SpellAOEBurst.generated.h"

class AAOEBurstBase;

/**
 * Spawns an expanding AOE shockwave centered on the caster,
 * damaging every enemy the ring passes through exactly once.
 */
UCLASS(Blueprintable, EditInlineNew)
class LOWPOLY_API USpellAOEBurst : public USpellBase
{
	GENERATED_BODY()

public:
	USpellAOEBurst();

	virtual bool Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|AOEBurst")
	TSubclassOf<AAOEBurstBase> BurstClass;

	/** Damage applied to each target caught in the burst */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|AOEBurst")
	FDamageBundle BurstDamage;

	/** Max radius the burst expands to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|AOEBurst")
	float MaxRadius = 600.0f;

	/** How quickly the burst radius expands (units per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|AOEBurst")
	float ExpansionSpeed = 500.0f;

	/** Vertical half-height filter for damage — keeps it grounded */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|AOEBurst")
	float DamageHalfHeight = 120.0f;
};
