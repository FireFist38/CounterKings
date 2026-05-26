#pragma once

#include "CoreMinimal.h"
#include "RangedWeaponBase.h"
#include "RangedBeamWeaponBase.generated.h"

UCLASS(Abstract)
class LOWPOLY_API ARangedBeamWeaponBase : public ARangedWeaponBase
{
	GENERATED_BODY()

public:
	ARangedBeamWeaponBase();

    virtual void StartAutoFire(AActor* Shooter) override;
    virtual void StopAutoFire() override;
    virtual void Fire(AActor* Shooter) override;
    virtual void Multicast_OnFired_Implementation() override;

protected:
	UPROPERTY(EditAnywhere, Category = "CK|Beam")
	TSubclassOf<class ABeamBase> BeamActorClass;

	UPROPERTY(EditAnywhere, Category = "CK|Beam")
	float BeamRange = 2500.0f;

    // How often the beam applies damage (seconds)
    UPROPERTY(EditAnywhere, Category = "CK|Beam")
    float DamageTickInterval = 0.1f;

	UPROPERTY()
	class ABeamBase* ActiveBeam;

    virtual void AutoFireTick() override;
    virtual bool FireSingleShot(AActor* Shooter) override;

    void UpdateBeamState(AActor* Shooter);

private:
    float LastDamageTime = 0.0f;
};
