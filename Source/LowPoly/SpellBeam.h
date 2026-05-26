#pragma once

#include "CoreMinimal.h"
#include "SpellChannelBase.h"
#include "ABeamBase.h"
#include "ItemBase.h"
#include "SpellBeam.generated.h"

UCLASS(Blueprintable, EditInlineNew)
class LOWPOLY_API USpellBeam : public USpellChannelBase
{
	GENERATED_BODY()

public:
	USpellBeam();

	// Beam visual actor class (use a Niagara-flavored BP)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	TSubclassOf<ABeamBase> BeamActorClass;

	// Damage dealt every tick while the beam hits a target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	FDamageBundle DamagePerTick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	float MaxRange = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell")
	bool bDrawDebugHit = false;

	virtual bool StartChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff) override;
	virtual bool TickChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff, float DeltaTime) override;
    virtual void EndChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff) override;

private:
    UPROPERTY()
    ABeamBase* ActiveBeam = nullptr;
};
