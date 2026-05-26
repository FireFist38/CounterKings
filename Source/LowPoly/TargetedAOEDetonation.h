#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.h"
#include "TargetedAOEDetonation.generated.h"

class UNiagaraComponent;
class USoundBase;
class UAttributeComponent;

/**
 * The actual effect that fires at the confirmed target location.
 * Deals damage in a radius instantly upon spawn, plays VFX, then destroys itself.
 */
UCLASS()
class LOWPOLY_API ATargetedAOEDetonation : public AActor
{
	GENERATED_BODY()

public:
	ATargetedAOEDetonation();

	virtual void Tick(float DeltaTime) override;

	void InitDetonation(AActor* InCaster, const FDamageBundle& InDamage);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Detonation")
	float DamageRadius = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Detonation")
	float DamageHalfHeight = 200.0f;

	/** How long the detonation actor lingers for VFX to play before destroying */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Detonation")
	float DetonationLifetime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Detonation|Effects")
	USoundBase* DetonationSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Detonation|Effects")
	class UNiagaraSystem* DetonationHitVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Detonation|Debug")
	bool bDebugDamage = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Detonation|Components")
	UNiagaraComponent* DetonationVFXComponent;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Applies damage in radius on the server, then multicasts VFX to all clients */
	void ApplyDamage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDetonationVFX();

	UPROPERTY()
	AActor* CasterActor = nullptr;

	FDamageBundle DamageBundle;

	FTimerHandle LifetimeTimerHandle;
};
