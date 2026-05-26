#include "FissureBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"

AFissureBase::AFissureBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(DamageRadius);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	FissureVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FissureVFXComponent"));
	FissureVFXComponent->SetupAttachment(CollisionSphere);
	FissureVFXComponent->bAutoActivate = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = MovementSpeed;
	ProjectileMovement->MaxSpeed = MovementSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->Velocity = FVector::ForwardVector * MovementSpeed; 
}

void AFissureBase::BeginPlay()
{
	Super::BeginPlay();
	// Timers and VFX are started in InitFissure() so the spell can set its values first.
}

void AFissureBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void AFissureBase::InitFissure(AActor* InCaster, const FDamageBundle& InDamage)
{
	CasterActor = InCaster;
	DamageBundle = InDamage;

	CollisionSphere->SetSphereRadius(DamageRadius);

	const FVector Dir = GetActorForwardVector();
	ProjectileMovement->Velocity = Dir * MovementSpeed;
	
	ProjectileMovement->SetComponentTickEnabled(true);
	ProjectileMovement->Activate(true);

	// Fire VFX on server AND all clients via multicast
	Multicast_ActivateVFX();

	// Start lifetime timer now that the Spell has overridden the Lifetime variable
	if (HasAuthority() && Lifetime > 0.0f)
	{
		GetWorldTimerManager().SetTimer(LifetimeTimerHandle, [this]()
		{
			Destroy();
		}, Lifetime, false);
	}
}

void AFissureBase::Multicast_ActivateVFX_Implementation()
{
	FissureVFXComponent->Activate(true);
}

void AFissureBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority()) return;

	ProjectileMovement->MaxSpeed = MovementSpeed;
	if (!ProjectileMovement->Velocity.IsNearlyZero())
	{
		ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetSafeNormal() * MovementSpeed;
	}

	FollowGround();
	CheckOverlaps();
}

void AFissureBase::FollowGround()
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

void AFissureBase::CheckOverlaps()
{
	if (!CasterActor) return;

	if (bDebugDamage)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 16, FColor::Orange, false, -1.0f);
	}

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
			if (!Actor || DamagedActors.Contains(Actor)) continue;

			UAttributeComponent* TargetAttr = Actor->FindComponentByClass<UAttributeComponent>();
			if (!TargetAttr || TargetAttr->IsDead()) continue;

			if (TargetAttr->ApplyCombatDamage(DamageBundle, CasterActor))
			{
				DamagedActors.Add(Actor);

				if (FissureHitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, FissureHitSound, Actor->GetActorLocation());
				}
				if (FissureHitVFX)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						GetWorld(), FissureHitVFX, Actor->GetActorLocation(),
						FRotator::ZeroRotator, FVector(1.0f), true, true, ENCPoolMethod::None, true);
				}
			}
		}
	}
}
