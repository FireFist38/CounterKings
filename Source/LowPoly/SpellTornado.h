#pragma once

#include "CoreMinimal.h"
#include "SpellBase.h"
#include "SpellTornado.generated.h"

class ATornadoBase;

/**
 * Summons a ground-gliding tornado that homes toward the nearest enemy,
 * dealing damage ticks to anyone inside its radius.
 */
UCLASS(Blueprintable, EditInlineNew)
class LOWPOLY_API USpellTornado : public USpellBase
{
	GENERATED_BODY()

public:
	USpellTornado();

	virtual bool Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Tornado")
	TSubclassOf<ATornadoBase> TornadoClass;

	/** Damage dealt per tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Tornado")
	FDamageBundle TornadoDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Tornado")
	float MovementSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Tornado")
	float Lifetime = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Tornado")
	float DamageRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Tornado")
	float HomingStrength = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Tornado")
	float MaxHomingDistance = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Tornado")
	float GroundFollowHeight = 50.0f;

	/** Distance in front of the caster to spawn the tornado */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Tornado")
	float SpawnDistance = 400.0f;
};