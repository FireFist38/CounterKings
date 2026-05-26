#pragma once

#include "CoreMinimal.h"
#include "SpellBase.h"
#include "SpellRisingWall.generated.h"

class ARisingWallBase;

UCLASS(Blueprintable, EditInlineNew)
class LOWPOLY_API USpellRisingWall : public USpellBase
{
	GENERATED_BODY()

public:
	USpellRisingWall();

	virtual bool Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	TSubclassOf<ARisingWallBase> WallClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SpawnDistance = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float GroundTraceHalfHeight = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float WallLifetime = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float WallRiseDuration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float WallRiseHeight = 180.0f;
};
