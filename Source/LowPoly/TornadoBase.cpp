#include "TornadoBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "DrawDebugHelpers.h"

ATornadoBase::ATornadoBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(DamageRadius);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetGenerateOverlapEvents(true);

	TornadoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TornadoMesh"));
	TornadoMesh->SetupAttachment(CollisionSphere);
	TornadoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 0.0f;   // We control speed manually in Tick
	ProjectileMovement->MaxSpeed = MovementSpeed;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.0f;
	ProjectileMovement->Friction = 0.0f;
	ProjectileMovement->Velocity = FVector::ZeroVector;
	ProjectileMovement->ProjectileGravityScale = 0.0f;  // We handle ground snapping

	LoopAudioComponent = nullptr;
}

void ATornadoBase::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	// Set up damage tick timer
	GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &ATornadoBase::ApplyDamageTick, DamageTickRate, true);

	// Set up lifetime destroy
	GetWorldTimerManager().SetTimer(LifetimeTimerHandle, [this]()
	{
		Destroy();
	}, Lifetime, false);
}

void ATornadoBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up looping audio
	if (LoopAudioComponent && LoopAudioComponent->IsPlaying())
	{
		LoopAudioComponent->FadeOut(0.3f, 0.0f);
	}

	GetWorldTimerManager().ClearTimer(DamageTimerHandle);
	GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);

	Super::EndPlay(EndPlayReason);
}

void ATornadoBase::InitTornado(AActor* InCaster, const FDamageBundle& InDamage, AActor* InHomingTarget)
{
	CasterActor = InCaster;
	DamagePerTick = InDamage;
	HomingTarget = InHomingTarget;

	// Set the collision radius
	CollisionSphere->SetSphereRadius(DamageRadius);

	// Set initial velocity direction toward homing target (or caster forward)
	if (HomingTarget)
	{
		const FVector Dir = (HomingTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		ProjectileMovement->Velocity = Dir * MovementSpeed;
	}
	else if (CasterActor)
	{
		const FVector Dir = CasterActor->GetActorForwardVector();
		ProjectileMovement->Velocity = Dir * MovementSpeed;
	}

	// Spawn Niagara VFX
	if (TornadoVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), TornadoVFX, GetActorLocation(), GetActorRotation(),
			FVector(1.0f), true, true, ENCPoolMethod::None, true);
	}

	// Start loop sound
	if (TornadoLoopSound)
	{
		LoopAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
			this, TornadoLoopSound, GetActorLocation());
	}
}

void ATornadoBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority()) return;

	// 1. Homing: steer toward target
	UpdateHoming(DeltaTime);

	// 2. Move: manually apply velocity (ProjectileMovement handles collision/bounce)
	ProjectileMovement->MaxSpeed = MovementSpeed;
	if (!ProjectileMovement->Velocity.IsNearlyZero())
	{
		ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetSafeNormal() * MovementSpeed;
	}

	// 3. Ground follow: snap to terrain
	FollowGround();

	// 4. Update loop audio location
	if (LoopAudioComponent && LoopAudioComponent->IsPlaying())
	{
		LoopAudioComponent->SetWorldLocation(GetActorLocation());
	}

	// 5. Self-rotate for visual effect (optional)
	AddActorWorldRotation(FRotator(0.0f, DeltaTime * 120.0f, 0.0f));
}

void ATornadoBase::UpdateHoming(float DeltaTime)
{
	if (!HomingTarget || !IsValid(HomingTarget))
	{
		// Target died or left the game — continue in current direction
		return;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), HomingTarget->GetActorLocation());
	if (DistSq > FMath::Square(MaxHomingDistance))
	{
		return; // Too far — stop homing
	}

	const FVector DesiredDir = (HomingTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	const FVector CurrentDir = ProjectileMovement->Velocity.GetSafeNormal();

	// Interpolate velocity direction toward target
	const FVector NewDir = FMath::VInterpTo(CurrentDir, DesiredDir, DeltaTime, HomingStrength);

	// Apply the NEW direction to velocity (speed is clamped to MovementSpeed in Tick)
	ProjectileMovement->Velocity = NewDir * MovementSpeed;
}

void ATornadoBase::FollowGround()
{
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0.0f, 0.0f, 500.0f); // Trace 500 units down

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (CasterActor) Params.AddIgnoredActor(CasterActor);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		// Found ground — lift the tornado to GroundFollowHeight above it
		FVector NewLoc = GetActorLocation();
		NewLoc.Z = Hit.ImpactPoint.Z + GroundFollowHeight;
		SetActorLocation(NewLoc);
	}
}

void ATornadoBase::ApplyDamageTick()
{
	if (!HasAuthority() || !CasterActor) return;

	DamagedThisTick.Empty();

	// Get all overlapping actors within the collision sphere
	TArray<AActor*> OverlappingActors;
	CollisionSphere->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor || Actor == CasterActor || DamagedThisTick.Contains(Actor)) continue;

		UAttributeComponent* TargetAttr = Actor->FindComponentByClass<UAttributeComponent>();
		if (!TargetAttr || TargetAttr->IsDead()) continue;

		if (TargetAttr->ApplyCombatDamage(DamagePerTick, CasterActor))
		{
			DamagedThisTick.Add(Actor);

			// Hit sound / VFX
			if (TornadoHitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, TornadoHitSound, Actor->GetActorLocation());
			}
			if (TornadoHitVFX)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(), TornadoHitVFX, Actor->GetActorLocation(),
					FRotator::ZeroRotator, FVector(1.0f), true, true, ENCPoolMethod::None, true);
			}
		}
	}
}