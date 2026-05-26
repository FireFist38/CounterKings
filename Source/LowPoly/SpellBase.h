#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SpellBase.generated.h"

class APlayerCharacter;
class AMagicWeaponBase;

// Abstract base for all spells. Subclasses override Cast() with their behavior.
// Spells are Instanced UObjects owned by a magic weapon — set the spell class in BP
// and tweak per-spell values (cost, cooldown, projectile class, etc.) inline.
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class LOWPOLY_API USpellBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	FName DisplayName = FName("Unnamed Spell");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	class UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	float ManaCost = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	float StaminaCost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	bool bUseMana = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	float Cooldown = 0.5f;

	// Delay before the spell effect fires after casting starts (lets the cast animation play first).
	// Mana, cooldown, and montage trigger immediately; only the actual spell effect is delayed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	float CastTime = 0.0f;

	// Server-authoritative cast. Subclasses implement actual effect.
	// Returns true if the spell was successfully cast (e.g., for cooldown tracking by the caller).
	virtual bool Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff);

protected:
	// Helper: get the staff's muzzle-equivalent location in world space.
	FVector GetCastOrigin(AMagicWeaponBase* Staff) const;

	// Helper: get the caster's view rotation (where the crosshair is aimed).
	FRotator GetCastRotation(APlayerCharacter* Caster) const;
};
