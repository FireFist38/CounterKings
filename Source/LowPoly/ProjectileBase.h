#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "NiagaraSystem.h"
#include "ItemBase.h"
#include "ProjectileBase.generated.h"

UCLASS()
class LOWPOLY_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase();

	void InitProjectile(AActor* InShooter, const FDamageBundle& InDamage, AActor* InAdditionalIgnoredActor = nullptr);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Projectile")
	class USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Projectile")
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile")
	float InitialSpeed = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile")
	float GravityScale = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile")
	float LifeSpan = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile")
	class USoundBase* ImpactSound;

	// Looping sound that plays while the projectile is in flight (auto-stops on impact/destroy)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile")
	class USoundBase* FlightSound;

	// Seconds the flight sound takes to fade out on impact (0 = hard cut)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile")
	float FlightFadeOutDuration = 1.5f;

	// Override attenuation for the flight sound; if null, uses the sound asset's default
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile")
	class USoundAttenuation* FlightSoundAttenuation;

	UPROPERTY()
	class UAudioComponent* FlightAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile")
	class UNiagaraSystem* ImpactEffect;

	// --- AoE on impact ---
	// If > 0, projectile applies splash damage to all actors within this radius on impact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile|AoE")
	float ExplosionRadius = 0.0f;

	// Falloff curve: 1.0 = full damage at center, 0.0 at edge. 0 = no falloff (full damage everywhere in radius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile|AoE", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ExplosionFalloff = 0.5f;

	// Niagara effect spawned at the explosion center (separate from regular impact effect)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile|AoE")
	class UNiagaraSystem* ExplosionEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile|AoE")
	class USoundBase* ExplosionSound;

	// Draws a debug sphere of the explosion radius at impact for tuning. Disable for shipping.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile|AoE")
	bool bDrawAoEDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Projectile|AoE")
	float AoEDebugDuration = 2.0f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	           FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnImpact(FVector ImpactPoint, FVector ImpactNormal);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnExplode(FVector Center);

	void ApplyAoeDamage(const FVector& Center);

	UFUNCTION(BlueprintImplementableEvent, Category = "CK|Projectile")
	void BP_OnImpact(FVector ImpactPoint, FVector ImpactNormal);

private:
	FDamageBundle DamageBundle;

	UPROPERTY()
	AActor* ShooterActor = nullptr;
};
