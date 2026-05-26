#include "SpellTornado.h"
#include "TornadoBase.h"
#include "PlayerCharacter.h"
#include "MagicWeaponBase.h"
#include "AttributeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

USpellTornado::USpellTornado()
{
	DisplayName = FName("Tornado");
	ManaCost = 30.0f;
	StaminaCost = 0.0f;
	bUseMana = true;
	Cooldown = 8.0f;
	CastTime = 0.0f;

	// Default damage: mix of Physical (wind) + Magic
	TornadoDamage.Physical = 8.0f;
	TornadoDamage.Magic = 4.0f;
}

bool USpellTornado::Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff)
{
	if (!Caster || !TornadoClass) return false;

	UWorld* World = Caster->GetWorld();
	if (!World) return false;

	// Step 1: Find the nearest enemy player for homing
	AActor* HomingTarget = nullptr;
	float ClosestDistSq = FMath::Square(MaxHomingDistance);

	// Iterate all player characters to find the closest enemy
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		APlayerCharacter* OtherPlayer = Cast<APlayerCharacter>(PC->GetPawn());
		if (!OtherPlayer || OtherPlayer == Caster) continue;

		UAttributeComponent* OtherAttr = OtherPlayer->GetAttributeComponent();
		if (!OtherAttr || OtherAttr->IsDead()) continue;

		const float DistSq = FVector::DistSquared(Caster->GetActorLocation(), OtherPlayer->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			HomingTarget = OtherPlayer;
		}
	}

	// Step 2: Determine spawn position — in front of caster, traced to ground
	const FRotator AimRot = Caster->GetControlRotation();
	const FRotator YawOnly(0.0f, AimRot.Yaw, 0.0f);
	const FVector SpawnOrigin = Caster->GetActorLocation() + (YawOnly.Vector() * SpawnDistance);

	// Trace down to find ground
	FVector SpawnPos = SpawnOrigin;
	{
		FHitResult GroundHit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Caster);

		const FVector TraceStart = SpawnOrigin + FVector(0.0f, 0.0f, 300.0f);
		const FVector TraceEnd = SpawnOrigin - FVector(0.0f, 0.0f, 500.0f);

		if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			SpawnPos = GroundHit.ImpactPoint;
		}
	}

	// Step 3: Spawn the tornado actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Caster;
	SpawnParams.Instigator = Caster;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ATornadoBase* Tornado = World->SpawnActor<ATornadoBase>(TornadoClass, SpawnPos, FRotator::ZeroRotator, SpawnParams);
	if (!Tornado) return false;

	// Step 4: Scale damage through attributes
	UAttributeComponent* Attr = Caster->GetAttributeComponent();
	FDamageBundle ScaledDamage = TornadoDamage;
	if (Staff && Attr)
	{
		ScaledDamage.Scale(Staff->ComputeAttributeMultiplier(Attr));
	}

	// Step 5: Configure the tornado
	Tornado->MovementSpeed = MovementSpeed;
	Tornado->Lifetime = Lifetime;
	Tornado->DamageRadius = DamageRadius;
	Tornado->DamageTickRate = 0.25f;
	Tornado->HomingStrength = HomingStrength;
	Tornado->MaxHomingDistance = MaxHomingDistance;
	Tornado->GroundFollowHeight = GroundFollowHeight;

	Tornado->InitTornado(Caster, ScaledDamage, HomingTarget);

	return true;
}