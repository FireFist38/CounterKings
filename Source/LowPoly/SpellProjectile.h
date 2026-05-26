#pragma once

#include "CoreMinimal.h"
#include "SpellBase.h"
#include "ItemBase.h"
#include "SpellProjectile.generated.h"

class AProjectileBase;

// Fires a magic projectile from the staff's CastSocket toward the caster's crosshair.
UCLASS(Blueprintable, EditInlineNew)
class LOWPOLY_API USpellProjectile : public USpellBase
{
	GENERATED_BODY()

public:
	USpellProjectile();

	// Projectile actor class to spawn (use a Niagara-flavored projectile BP for visuals)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	TSubclassOf<AProjectileBase> ProjectileClass;

    /** Base damage bundle for this spell (scaled by caster attributes via staff) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	FDamageBundle SpellDamage;

	virtual bool Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff) override;
};
