#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.h"
#include "TornadoBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class USoundBase;
class UAudioComponent;

/**
 * A slow-moving tornado that glides across the ground,
 * homing toward the nearest enemy and dealing damage ticks to anyone inside.
 */
UCLASS()
class LOWPOLY_API ATornadoBase : public AActor
{
	GENERATED_BODY()

public:
	ATornadoBase();

	virtual void Tick(float DeltaTime) override;

	/**
	 * Initialize the tornado with damage data and optional homing target.
	 */
	void InitTornado(AActor* InCaster, const FDamageBundle& InDamage, AActor* InHomingTarget);

	// --- Config (set by the spell before InitTornado is called) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tornado")
	float Lifetime = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tornado")
	float MovementSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tornado")
	float GroundFollowHeight = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tornado")
	float DamageRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tornado")
	float DamageTickRate = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tornado")
	float HomingStrength = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tornado")
	float MaxHomingDistance = 3000.0f;

	// Weaker homing applied strictly to forward direction (desired vs actual velocity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tornado")
	float ForwardHomingWeight = 0.03f;

	// --- Audio & VFX ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tornado|Effects")
	USoundBase* TornadoLoopSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tornado|Effects")
	USoundBase* TornadoHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tornado|Effects")
	class UNiagaraSystem* TornadoVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tornado|Effects")
	class UNiagaraSystem* TornadoHitVFX;

	// --- Debug ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Tornado|Debug")
	bool bDebugDamage = false;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Overlap sphere for damage detection */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Tornado|Components")
	USphereComponent* CollisionSphere;

	/** Visual mesh (optional — Niagara may handle visuals instead) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Tornado|Components")
	UStaticMeshComponent* TornadoMesh;

	/** Moves the tornado forward */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Tornado|Components")
	UProjectileMovementComponent* ProjectileMovement;

	/** VFX Component so it follows the mesh and dies with it */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Tornado|Components")
	UNiagaraComponent* TornadoVFXComponent;

	UPROPERTY()
	UAudioComponent* LoopAudioComponent;

private:
	/** Called on a timer to apply damage to overlapping actors */
	UFUNCTION()
	void ApplyDamageTick();

	/** Trace downward to snap the tornado to the ground */
	void FollowGround();

	/** Steer toward the homing target (if valid) */
	void UpdateHoming(float DeltaTime);

	/** Fires VFX & audio on server + all clients */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ActivateVFX();

	UPROPERTY()
	AActor* CasterActor = nullptr;

	UPROPERTY()
	AActor* HomingTarget = nullptr;

	FDamageBundle DamagePerTick;

	FTimerHandle LifetimeTimerHandle;
	FTimerHandle DamageTimerHandle;

	/** Actors already damaged this tick (cleared each tick) */
	TSet<AActor*> DamagedThisTick;
};