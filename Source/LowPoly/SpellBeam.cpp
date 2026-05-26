#include "SpellBeam.h"
#include "PlayerCharacter.h"
#include "MagicWeaponBase.h"
#include "AttributeComponent.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"

USpellBeam::USpellBeam()
{
	DisplayName = FName("Channeled Beam");
    DamagePerTick.Magic = 5.0f;
}

bool USpellBeam::StartChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff)
{
    // Super calls Caster->SetRotationClamp(true)
	if (!Super::StartChannel(Caster, Staff) || !BeamActorClass)
	{
		return false;
	}

	FActorSpawnParameters Params;
	Params.Owner = Caster;

	// Spawn at the cast socket world position. The beam actor will move itself
	// every frame in its Tick — do NOT attach it to the socket.
	const FVector  SpawnOrigin   = GetCastOrigin(Staff);
	const FRotator SpawnRotation = GetCastRotation(Caster);

	ActiveBeam = Caster->GetWorld()->SpawnActor<ABeamBase>(
		BeamActorClass, SpawnOrigin, SpawnRotation, Params);

	if (ActiveBeam)
	{
		// Give the beam actor a reference to the staff mesh and socket name so
		// its Tick can query the socket location every frame for smooth tracking.
		if (UStaticMeshComponent* SM = Staff->FindComponentByClass<UStaticMeshComponent>())
		{
			ActiveBeam->InitBeamTracking(SM, Staff->GetCastSocketName());
		}
	}

	return ActiveBeam != nullptr;
}

bool USpellBeam::TickChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff, float DeltaTime)
{
	if (!Super::TickChannel(Caster, Staff, DeltaTime))
	{
		return false;
	}

	if (!Caster || !ActiveBeam)
	{
		UE_LOG(LogTemp, Warning, TEXT("USpellBeam::TickChannel - Caster or ActiveBeam is null."));
		return false;
	}

	// --- Line trace from the CAMERA so the beam follows the crosshair exactly ---
	// The Niagara beam visually starts at the cast socket (ABeamBase::Tick moves
	// the actor there), but aiming direction comes from the camera/crosshair.
	FVector TraceStart;
	const FRotator TraceRotation = GetCastRotation(Caster);

	if (UCameraComponent* Cam = Caster->FindComponentByClass<UCameraComponent>())
	{
		TraceStart = Cam->GetComponentLocation();
	}
	else
	{
		// Fallback to cast socket if no camera found.
		TraceStart = GetCastOrigin(Staff);
	}

	const FVector TraceEnd = TraceStart + (TraceRotation.Vector() * MaxRange);

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Caster);
	QueryParams.AddIgnoredActor(Staff);

	const bool bHit = Caster->GetWorld()->LineTraceSingleByChannel(
		Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	// Socket world position = visual start of the beam.
	// Crosshair trace result = visual end of the beam.
	const FVector SocketStart  = GetCastOrigin(Staff);
	const FVector BeamEndPoint = bHit ? Hit.ImpactPoint : TraceEnd;

	// Push start + end to the beam actor. The actor will reposition itself to
	// SocketStart, rotate to face BeamEndPoint, and update Niagara params.
	ActiveBeam->UpdateBeam(SocketStart, BeamEndPoint, bHit, Hit);

	if (bDrawDebugHit)
	{
		DrawDebugLine(Caster->GetWorld(), SocketStart, BeamEndPoint,
			bHit ? FColor::Red : FColor::Green, false, TickRate);
		if (bHit)
			DrawDebugSphere(Caster->GetWorld(), Hit.ImpactPoint, 10.0f, 8, FColor::Red, false, TickRate);
	}

	// Apply damage on tick.
	if (bHit && Hit.GetActor())
	{
		if (UAttributeComponent* TargetAttr = Hit.GetActor()->FindComponentByClass<UAttributeComponent>())
		{
			const float Mult = Staff ? Staff->ComputeAttributeMultiplier(Caster->GetAttributeComponent()) : 1.0f;
            FDamageBundle ScaledDamage = DamagePerTick;
            ScaledDamage.Scale(Mult);
            
			TargetAttr->ApplyCombatDamage(ScaledDamage, Caster);
		}
	}

	return true;
}

void USpellBeam::EndChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff)
{
	if (ActiveBeam)
	{
		ActiveBeam->Destroy();
		ActiveBeam = nullptr;
	}

    // Super calls Caster->SetRotationClamp(false)
	Super::EndChannel(Caster, Staff);
}
