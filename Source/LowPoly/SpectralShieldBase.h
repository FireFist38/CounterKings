#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpectralShieldBase.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;
struct FDamageBundle;

/**
 * Physical entity for the Spectral Shield spell.
 * Blocks projectiles and redirects incoming damage to mana drain.
 * Positioned in front of the caster, tracking YAW rotation only (like Reinhardt/Dr. Strange barrier).
 */
UCLASS(Blueprintable, BlueprintType)
class LOWPOLY_API ASpectralShieldBase : public AActor
{
	GENERATED_BODY()

public:
	ASpectralShieldBase();

	/** Called by ApplyCombatDamage when damage hits this shield. Returns true if damage was absorbed. */
	bool AbsorbDamage(AActor* DamageInstigator, const FDamageBundle& Damage);

	/** Configures the shield visuals and collision extents. Optional override. */
	UFUNCTION(BlueprintCallable, Category = "CK|Shield")
	void ConfigureShield(const FVector& BoxExtent);

	/** Ensures the shield doesn't collide with the person holding it */
	UFUNCTION(BlueprintCallable, Category = "CK|Shield")
	void SetCaster(AActor* Caster);

	/** Returns the caster (the player who summoned this shield) */
	UFUNCTION(BlueprintCallable, Category = "CK|Shield")
	AActor* GetCaster() const { return CasterActor; }

	/** Reference to the caster player character */
	UPROPERTY()
	AActor* CasterActor = nullptr;

	// --- Impact FX (plays when the shield absorbs damage) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Shield|Effects")
	USoundBase* ShieldHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Shield|Effects")
	UNiagaraSystem* ShieldHitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Shield|Effects")
	float ShieldHitEffectScale = 1.0f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnShieldHit();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Shield")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Shield")
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Shield")
	UStaticMeshComponent* ShieldMesh;
};