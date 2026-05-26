#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.h"
#include "TargetedAOEIndicator.generated.h"

class UNiagaraComponent;

/**
 * A cosmetic targeting indicator that follows the caster's crosshair aim point on the ground.
 * Ticks on the server and locally-controlled client for smooth tracking.
 * Position replication ensures the server has an accurate location for detonation.
 */
UCLASS()
class LOWPOLY_API ATargetedAOEIndicator : public AActor
{
	GENERATED_BODY()

public:
	ATargetedAOEIndicator();

	virtual void Tick(float DeltaTime) override;

	/** Called by the spell after spawn to activate visuals on all clients */
	void InitIndicator(float InRadius, float InMaxRange);

	/** Visual radius of the targeting circle — should match the detonation damage radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Indicator")
	float IndicatorRadius = 400.0f;

	/** Max range of the ground trace from the camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Indicator")
	float MaxTargetRange = 2000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Indicator|Components")
	UNiagaraComponent* IndicatorVFXComponent;

protected:
	virtual void BeginPlay() override;

private:
	/** Broadcasts VFX activation to all clients */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ActivateVFX();

	/** Finds the ground point under the caster's crosshair */
	bool GetAimGroundPoint(FVector& OutPoint) const;
};
