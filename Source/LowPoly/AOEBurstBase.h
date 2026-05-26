#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.h"
#include "AOEBurstBase.generated.h"

class USphereComponent;
class UNiagaraComponent;
class USoundBase;
class UAttributeComponent;

/**
 * An expanding shockwave actor that originates at a point, expands to MaxRadius,
 * dealing damage once to every enemy the expanding ring touches.
 */
UCLASS()
class LOWPOLY_API AAOEBurstBase : public AActor
{
	GENERATED_BODY()

public:
	AAOEBurstBase();

	virtual void Tick(float DeltaTime) override;

	void InitBurst(AActor* InCaster, const FDamageBundle& InDamage);

	// --- Config ---

	/** Radius the burst expands to before self-destructing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|AOEBurst")
	float MaxRadius = 600.0f;

	/** How fast the burst radius grows (units per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|AOEBurst")
	float ExpansionSpeed = 500.0f;

	/** Vertical half-height of the damage check — keeps the sweep close to the ground */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|AOEBurst")
	float DamageHalfHeight = 120.0f;

	// --- Effects ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|AOEBurst|Effects")
	USoundBase* BurstHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|AOEBurst|Effects")
	class UNiagaraSystem* BurstHitVFX;

	// --- Debug ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|AOEBurst|Debug")
	bool bDebugDamage = false;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** VFX component — attach your expanding ring/shockwave Niagara system here */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|AOEBurst|Components")
	UNiagaraComponent* BurstVFXComponent;

private:
	void ExpandAndCheck(float DeltaTime);

	/** Fires VFX on server + all clients */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ActivateVFX();

	UPROPERTY()
	AActor* CasterActor = nullptr;

	FDamageBundle DamageBundle;

	/** Current radius of the expanding burst this frame */
	float CurrentRadius = 0.0f;

	/** Actors already hit — ensures each enemy is damaged exactly once */
	TSet<AActor*> DamagedActors;
};
