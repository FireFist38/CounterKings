#pragma once

#include "CoreMinimal.h"
#include "SpellBase.h"
#include "ItemBase.h"
#include "SpellFissure.generated.h"

class AFissureBase;

/**
 * Creates a fissure/rock splinter that glides quickly over the ground in a straight line,
 * piercing through and damaging enemies in its path once.
 */
UCLASS(Blueprintable, EditInlineNew)
class LOWPOLY_API USpellFissure : public USpellBase
{
	GENERATED_BODY()

public:
	USpellFissure();

	virtual bool Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Fissure")
	TSubclassOf<AFissureBase> FissureClass;

	/** Damage dealt once per target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Fissure")
	FDamageBundle FissureDamage;

	/** Number of fissures to spawn in an arc (e.g. 3 for a W shape/trident) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Fissure")
	int32 FissureCount = 3;

	/** Total angle of the spread arc in degrees (e.g., 90 means 45 degrees left and right) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Fissure")
	float SpreadAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Fissure")
	float MovementSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Fissure")
	float Lifetime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Fissure")
	float DamageRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Fissure")
	float GroundFollowHeight = 20.0f;

	/** Distance in front of the caster to spawn the fissure */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Fissure")
	float SpawnDistance = 200.0f;
};
