#pragma once

#include "CoreMinimal.h"
#include "SpellTargetedAOE.h"
#include "SpellIcicleDrop.generated.h"

class AProjectileBase;

/**
 * A targeted spell that drops an icicle (projectile) from the sky onto the target location.
 * Uses the Targeted AOE indicator for aiming.
 */
UCLASS(Blueprintable, EditInlineNew)
class LOWPOLY_API USpellIcicleDrop : public USpellTargetedAOE
{
	GENERATED_BODY()

public:
	USpellIcicleDrop();

	/** The projectile to drop from the sky */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Icicle")
	TSubclassOf<AProjectileBase> IcicleClass;

	/** Height above the ground indicator to spawn the icicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Icicle")
	float DropHeight = 1200.0f;

	/** Initial downward speed of the icicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Icicle")
	float InitialDropSpeed = 3000.0f;

protected:
	virtual void OnDetonate(APlayerCharacter* Caster, AMagicWeaponBase* Staff, const FVector& TargetLocation) override;
};
