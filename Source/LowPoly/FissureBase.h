#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.h"
#include "FissureBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class USoundBase;

UCLASS()
class LOWPOLY_API AFissureBase : public AActor
{
	GENERATED_BODY()

public:
	AFissureBase();

	virtual void Tick(float DeltaTime) override;

	void InitFissure(AActor* InCaster, const FDamageBundle& InDamage);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Fissure")
	float Lifetime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Fissure")
	float MovementSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Fissure")
	float GroundFollowHeight = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Fissure")
	float DamageRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Fissure|Effects")
	USoundBase* FissureHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Fissure|Effects")
	class UNiagaraSystem* FissureHitVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Fissure|Debug")
	bool bDebugDamage = false;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Fissure|Components")
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Fissure|Components")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Fissure|Components")
	UNiagaraComponent* FissureVFXComponent;

private:
	void FollowGround();
	void CheckOverlaps();

	/** Fires VFX on server + all clients */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ActivateVFX();

	UPROPERTY()
	AActor* CasterActor = nullptr;

	FDamageBundle DamageBundle;

	FTimerHandle LifetimeTimerHandle;

	TSet<AActor*> DamagedActors;
};
