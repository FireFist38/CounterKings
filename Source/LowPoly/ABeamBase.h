#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "ABeamBase.generated.h"

UCLASS()
class LOWPOLY_API ABeamBase : public AActor
{
	GENERATED_BODY()

public:
	ABeamBase();

	virtual void Tick(float DeltaTime) override;

	/**
	 * Called by USpellBeam on the server every spell tick.
	 * Moves this actor to Start, rotates it to face End, and pushes Niagara parameters.
	 */
	void UpdateBeam(FVector Start, FVector End, bool bHit, FHitResult Hit);

	/**
	 * Store a reference to the staff mesh and socket so this actor's Tick can
	 * reposition itself every FRAME (smoother than the spell TickRate interval).
	 */
	void InitBeamTracking(UStaticMeshComponent* StaffMesh, FName SocketName);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Beam")
	class USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Beam")
	class UNiagaraComponent* BeamComponent;

    // --- Audio & Visual FX (Synced with ProjectileBase style) ---

    // Looping sound that plays while the beam is active. Starts at the socket.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Beam|Audio")
	class USoundBase* BeamFlightSound;

    // Looping sound that plays at the IMPACT point. Only active if the beam hits something.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Beam|Audio")
	class USoundBase* BeamImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Beam|Audio")
	class USoundAttenuation* BeamSoundAttenuation;

    // Seconds the sounds take to fade out when the beam stops (0 = hard cut)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Beam|Audio")
	float SoundFadeOutDuration = 0.5f;

    // Niagara effect spawned at the impact point. Active only during a hit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Beam|VFX")
	class UNiagaraSystem* ImpactEffect;

    // Niagara component for the impact FX - kept as a member to move it every frame.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Beam|VFX")
    class UNiagaraComponent* ImpactComponent;

	// Niagara user parameter name for the beam START position (world space).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Beam")
	FName StartPointParam = FName("BeamStart");

	// Niagara user parameter name for the beam END position (world space).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Beam")
	FName EndPointParam = FName("BeamEnd");

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Replicated world-space endpoint. Used on clients to update Niagara and Audio params.
	UPROPERTY(ReplicatedUsing = OnRep_BeamEndLocation)
	FVector BeamEndLocation;

    // Replicated hit state.
    UPROPERTY(ReplicatedUsing = OnRep_bIsHitting)
    bool bIsHitting;

	UFUNCTION()
	void OnRep_BeamEndLocation();

    UFUNCTION()
    void OnRep_bIsHitting();

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Weak reference to the staff mesh — used in Tick to track socket position every frame.
	UPROPERTY()
	TWeakObjectPtr<UStaticMeshComponent> TrackingMesh;

	FName TrackingSocket;

    UPROPERTY()
    UAudioComponent* FlightAudio;

    UPROPERTY()
    UAudioComponent* ImpactAudio;

	// Repositions/rotates this actor and pushes Niagara params.
	void ApplyBeamTransformAndParams(FVector WorldStart, FVector WorldEnd);

    // Manages audio and impact VFX based on hit state.
    void UpdateDynamicEffects();
};
