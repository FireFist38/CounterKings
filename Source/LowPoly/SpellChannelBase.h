#pragma once

#include "CoreMinimal.h"
#include "SpellBase.h"
#include "SpellChannelBase.generated.h"

// Base for spells that need to tick over time while held
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class LOWPOLY_API USpellChannelBase : public USpellBase
{
	GENERATED_BODY()

public:
	// Mana or Stamina drained per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Channel")
	float DrainPerSecond = 10.0f;

	// How often (in seconds) the spell effect and resource drain tick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Channel")
	float TickRate = 0.1f;

    // Angle limit for rotation clamping during channel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Channel")
	float ChannelRotationClampAngle = 45.0f;

    virtual bool StartChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff);
    virtual bool TickChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff, float DeltaTime);
    virtual void EndChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff);
};
