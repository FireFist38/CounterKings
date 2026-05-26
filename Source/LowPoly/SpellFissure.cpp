#include "SpellFissure.h"
#include "FissureBase.h"
#include "PlayerCharacter.h"
#include "MagicWeaponBase.h"
#include "AttributeComponent.h"
#include "Engine/World.h"

USpellFissure::USpellFissure()
{
	DisplayName = FName("Fissure");
	ManaCost = 25.0f;
	StaminaCost = 0.0f;
	bUseMana = true;
	Cooldown = 4.0f;
	CastTime = 0.0f;

	FissureDamage.Physical = 15.0f;
	FissureDamage.Magic = 5.0f;
}

bool USpellFissure::Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff)
{
	if (!Caster || !FissureClass) return false;

	UWorld* World = Caster->GetWorld();
	if (!World) return false;

	// Scale damage once
	UAttributeComponent* Attr = Caster->GetAttributeComponent();
	FDamageBundle ScaledDamage = FissureDamage;
	if (Staff && Attr)
	{
		ScaledDamage.Scale(Staff->ComputeAttributeMultiplier(Attr));
	}

	// Base aim rotation
	const FRotator AimRot = Caster->GetControlRotation();
	const FRotator YawOnly(0.0f, AimRot.Yaw, 0.0f);

	// Calculate angle distribution
	float StartYaw = YawOnly.Yaw;
	float AngleStep = 0.0f;
	
	if (FissureCount > 1)
	{
		StartYaw = YawOnly.Yaw - (SpreadAngle / 2.0f);
		AngleStep = SpreadAngle / (FissureCount - 1);
	}

	bool bSpawnedAny = false;

	for (int32 i = 0; i < FissureCount; ++i)
	{
		FRotator SpawnRot = YawOnly;
		SpawnRot.Yaw = StartYaw + (i * AngleStep);

		const FVector SpawnOrigin = Caster->GetActorLocation() + (SpawnRot.Vector() * SpawnDistance);

		// Trace down to find ground
		FVector SpawnPos = SpawnOrigin;
		FHitResult GroundHit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Caster);

		const FVector TraceStart = SpawnOrigin + FVector(0.0f, 0.0f, 300.0f);
		const FVector TraceEnd = SpawnOrigin - FVector(0.0f, 0.0f, 500.0f);

		if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			SpawnPos = GroundHit.ImpactPoint;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Caster;
		SpawnParams.Instigator = Caster;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AFissureBase* Fissure = World->SpawnActor<AFissureBase>(FissureClass, SpawnPos, SpawnRot, SpawnParams);
		if (Fissure)
		{
			bSpawnedAny = true;

			Fissure->MovementSpeed = MovementSpeed;
			Fissure->Lifetime = Lifetime;
			Fissure->DamageRadius = DamageRadius;
			Fissure->GroundFollowHeight = GroundFollowHeight;

			Fissure->InitFissure(Caster, ScaledDamage);
		}
	}

	return bSpawnedAny;
}
