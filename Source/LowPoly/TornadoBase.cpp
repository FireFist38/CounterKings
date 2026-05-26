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
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"

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

	TornadoVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TornadoVFXComponent"));
	TornadoVFXComponent->SetupAttachment(CollisionSphere);
	TornadoVFXComponent->bAutoActivate = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = MovementSpeed;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.0f;
	ProjectileMovement->Friction = 0.0f;
	ProjectileMovement->Velocity = FVector::ZeroVector;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bAutoUpdateTickRegistration = false;

	LoopAudioComponent = nullptr;
}

void ATornadoBase::BeginPlay()
{
	Super::BeginPlay();
	// Timers and VFX are started in InitTornado() so the spell can set its values first.
}

void ATornadoBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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

	CollisionSphere->SetSphereRadius(DamageRadius);

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

	ProjectileMovement->SetComponentTickEnabled(true);
	ProjectileMovement->Activate(true);

	// Fire VFX + audio on server AND all clients via multicast
	Multicast_ActivateVFX();

	if (HasAuthority())
	{
		if (DamageTickRate > 0.0f)
		{
			GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &ATornadoBase::ApplyDamageTick, DamageTickRate, true);
		}

		if (Lifetime > 0.0f)
		{
			GetWorldTimerManager().SetTimer(LifetimeTimerHandle, [this]()
			{
				Destroy();
			}, Lifetime, false);
		}
	}
}

void ATornadoBase::Multicast_ActivateVFX_Implementation()
{
	if (TornadoVFX)
	{
		TornadoVFXComponent->SetAsset(TornadoVFX);
		TornadoVFXComponent->Activate(true);
	}

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

	UpdateHoming(DeltaTime);

	ProjectileMovement->MaxSpeed = MovementSpeed;
	if (!ProjectileMovement->Velocity.IsNearlyZero())
	{
		ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetSafeNormal() * MovementSpeed;
	}

	FollowGround();

	if (LoopAudioComponent && LoopAudioComponent->IsPlaying())
	{
		LoopAudioComponent->SetWorldLocation(GetActorLocation());
	}

	AddActorWorldRotation(FRotator(0.0f, DeltaTime * 120.0f, 0.0f));
}

void ATornadoBase::UpdateHoming(float DeltaTime)
{
	if (!HomingTarget || !IsValid(HomingTarget))
	{
		return;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), HomingTarget->GetActorLocation());
	if (DistSq > FMath::Square(MaxHomingDistance))
	{
		return;
	}

	const FVector DesiredDir = (HomingTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	const FVector CurrentDir = ProjectileMovement->Velocity.GetSafeNormal();
	const FVector NewDir = FMath::VInterpTo(CurrentDir, DesiredDir, DeltaTime, HomingStrength);
	ProjectileMovement->Velocity = NewDir * MovementSpeed;
}

void ATornadoBase::FollowGround()
{
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0.0f, 0.0f, 500.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (CasterActor) Params.AddIgnoredActor(CasterActor);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		FVector NewLoc = GetActorLocation();
		NewLoc.Z = Hit.ImpactPoint.Z + GroundFollowHeight;
		SetActorLocation(NewLoc);
	}
}

void ATornadoBase::ApplyDamageTick()
{
	if (!HasAuthority() || !CasterActor) return;

	if (bDebugDamage)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 32, FColor::Red, false, DamageTickRate);
	}

	DamagedThisTick.Empty();

	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(DamageRadius);
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Destructible);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(CasterActor);

	if (GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, ObjectQueryParams, Sphere, QueryParams))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* Actor = Overlap.GetActor();
			if (!Actor || DamagedThisTick.Contains(Actor)) continue;

			UAttributeComponent* TargetAttr = Actor->FindComponentByClass<UAttributeComponent>();
			if (!TargetAttr || TargetAttr->IsDead()) continue;

			if (TargetAttr->ApplyCombatDamage(DamagePerTick, CasterActor))
			{
				DamagedThisTick.Add(Actor);

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
}