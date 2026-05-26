#include "ProjectileBase.h"
#include "AttributeComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"

AProjectileBase::AProjectileBase()
{
	bReplicates = true;
	SetReplicateMovement(true);
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(10.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	CollisionSphere->SetNotifyRigidBodyCollision(true);
	SetRootComponent(CollisionSphere);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 5000.0f;
	ProjectileMovement->MaxSpeed = 5000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.1f;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bSweepCollision = true;
	ProjectileMovement->bForceSubStepping = true;
	ProjectileMovement->MaxSimulationTimeStep = 0.016f;
	ProjectileMovement->MaxSimulationIterations = 8;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	// Enforce collision settings and updated component at runtime to override any Blueprint serialization issues
	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
		CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
		CollisionSphere->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		
		// Guarantee we block static environment, character pawns, and test dummies
		CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		
		CollisionSphere->SetNotifyRigidBodyCollision(true);
	}

	if (ProjectileMovement && CollisionSphere)
	{
		ProjectileMovement->SetUpdatedComponent(CollisionSphere);
	}

	if (HasAuthority())
	{
		ProjectileMovement->InitialSpeed = InitialSpeed;
		ProjectileMovement->MaxSpeed = InitialSpeed;
		ProjectileMovement->ProjectileGravityScale = GravityScale;
		CollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
		SetLifeSpan(LifeSpan);
	}

	// Flight sound plays on all clients; auto-stops when the actor is destroyed (impact / lifespan)
	if (FlightSound)
	{
		FlightAudio = UGameplayStatics::SpawnSoundAttached(
			FlightSound,
			GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::SnapToTarget,
			true,   // bStopWhenAttachedToDestroyed
			1.0f, 1.0f, 0.0f,
			FlightSoundAttenuation,
			nullptr,
			false   // bAutoDestroy — we keep a handle so we can Stop() on impact
		);
	}
}

void AProjectileBase::InitProjectile(AActor* InShooter, const FDamageBundle& InDamage, AActor* InAdditionalIgnoredActor)
{
	ShooterActor = InShooter;
	DamageBundle = InDamage;

	if (ShooterActor)
	{
		CollisionSphere->MoveIgnoreActors.Add(ShooterActor);
	}

	if (InAdditionalIgnoredActor)
	{
		CollisionSphere->MoveIgnoreActors.Add(InAdditionalIgnoredActor);
	}

	// Ignore other active projectiles to prevent mid-air collisions
	if (GetWorld())
	{
		TArray<AActor*> FoundProjectiles;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AProjectileBase::StaticClass(), FoundProjectiles);
		for (AActor* Proj : FoundProjectiles)
		{
			if (Proj && Proj != this)
			{
				CollisionSphere->MoveIgnoreActors.Add(Proj);
				if (AProjectileBase* OtherProj = Cast<AProjectileBase>(Proj))
				{
					if (OtherProj->CollisionSphere)
					{
						OtherProj->CollisionSphere->MoveIgnoreActors.Add(this);
					}
				}
			}
		}
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = InitialSpeed;
		ProjectileMovement->MaxSpeed = InitialSpeed;
		ProjectileMovement->ProjectileGravityScale = GravityScale;
		ProjectileMovement->Velocity = GetActorForwardVector() * InitialSpeed;
		ProjectileMovement->UpdateComponentVelocity();
	}
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                             FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || !OtherActor || OtherActor == ShooterActor || OtherActor->IsA(AProjectileBase::StaticClass())) return;

	if (ExplosionRadius > 0.0f)
	{
		// AoE — apply splash damage to all actors within this radius on impact
		ApplyAoeDamage(Hit.ImpactPoint);
		Multicast_OnExplode(Hit.ImpactPoint);
	}
	else
	{
		// Single-target hit
		UAttributeComponent* TargetAttr = OtherActor->FindComponentByClass<UAttributeComponent>();
		if (TargetAttr && !TargetAttr->IsDead())
		{
			TargetAttr->ApplyCombatDamage(DamageBundle, ShooterActor);
		}
	}

	Multicast_OnImpact(Hit.ImpactPoint, Hit.ImpactNormal);
	
	// Disable collision and movement to prevent multiple hits while we wait for destruction
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	// Set a short lifespan to give the multicast RPC time to reach all clients before the actor is destroyed.
	SetLifeSpan(0.2f);
}

void AProjectileBase::ApplyAoeDamage(const FVector& Center)
{
	if (!HasAuthority() || ExplosionRadius <= 0.0f) return;

	if (bDrawAoEDebug)
	{
		DrawDebugSphere(GetWorld(), Center, ExplosionRadius, 24, FColor::Red, false, AoEDebugDuration, 0, 2.0f);
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(ExplosionRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Center,
		FQuat::Identity,
		FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllDynamicObjects),
		Sphere,
		Params);

	TSet<AActor*> DamagedActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target || Target == ShooterActor || DamagedActors.Contains(Target)) continue;

		UAttributeComponent* TargetAttr = Target->FindComponentByClass<UAttributeComponent>();
		if (!TargetAttr || TargetAttr->IsDead()) continue;

		DamagedActors.Add(Target);

		// Compute distance-based falloff multiplier
		const float Dist = FVector::Dist(Center, Target->GetActorLocation());
		const float Normalized = FMath::Clamp(Dist / ExplosionRadius, 0.0f, 1.0f);
		const float Mult = FMath::Lerp(1.0f, FMath::Max(1.0f - ExplosionFalloff, 0.0f), Normalized);

        FDamageBundle ScaledDamage = DamageBundle;
        ScaledDamage.Scale(Mult);

		TargetAttr->ApplyCombatDamage(ScaledDamage, ShooterActor);
	}
}

void AProjectileBase::Multicast_OnExplode_Implementation(FVector Center)
{
	if (ExplosionSound)
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, Center);

	if (ExplosionEffect)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ExplosionEffect, Center,
			FRotator::ZeroRotator, FVector(1.f), true, true, ENCPoolMethod::None, true);
}

void AProjectileBase::Multicast_OnImpact_Implementation(FVector ImpactPoint, FVector ImpactNormal)
{
	if (FlightAudio && FlightAudio->IsPlaying())
	{
		// Detach so the fade survives the projectile being destroyed
		FlightAudio->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		FlightAudio->bAutoDestroy = true;
		FlightAudio->FadeOut(FlightFadeOutDuration, 0.0f);
	}

	if (ImpactSound)
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, ImpactPoint);

	if (ImpactEffect)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, ImpactPoint,
			ImpactNormal.Rotation(), FVector(1.f), true, true, ENCPoolMethod::None, true);

	BP_OnImpact(ImpactPoint, ImpactNormal);
}
