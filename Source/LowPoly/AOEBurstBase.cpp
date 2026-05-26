#include "AOEBurstBase.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AttributeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"

AAOEBurstBase::AAOEBurstBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false); // Static — does not move

	BurstVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BurstVFXComponent"));
	SetRootComponent(BurstVFXComponent);
	BurstVFXComponent->bAutoActivate = false;
}

void AAOEBurstBase::BeginPlay()
{
	Super::BeginPlay();
}

void AAOEBurstBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AAOEBurstBase::InitBurst(AActor* InCaster, const FDamageBundle& InDamage)
{
	CasterActor = InCaster;
	DamageBundle = InDamage;
	CurrentRadius = 0.0f;

	// Fire VFX on server AND all clients via multicast
	Multicast_ActivateVFX();
}

void AAOEBurstBase::Multicast_ActivateVFX_Implementation()
{
	BurstVFXComponent->Activate(true);
}

void AAOEBurstBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only the server runs damage logic
	if (!HasAuthority()) return;

	ExpandAndCheck(DeltaTime);
}

void AAOEBurstBase::ExpandAndCheck(float DeltaTime)
{
	if (!CasterActor) return;

	CurrentRadius = FMath::Min(CurrentRadius + (ExpansionSpeed * DeltaTime), MaxRadius);

	if (bDebugDamage)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), CurrentRadius, 32, FColor::Cyan, false, -1.0f);
	}

	TArray<FOverlapResult> Overlaps;
	const FCollisionShape CurrentSphere = FCollisionShape::MakeSphere(CurrentRadius);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Destructible);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(CasterActor);

	if (GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, ObjectQueryParams, CurrentSphere, QueryParams))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* Actor = Overlap.GetActor();
			if (!Actor || DamagedActors.Contains(Actor)) continue;

			const float HeightDiff = FMath::Abs(Actor->GetActorLocation().Z - GetActorLocation().Z);
			if (HeightDiff > DamageHalfHeight) continue;

			UAttributeComponent* TargetAttr = Actor->FindComponentByClass<UAttributeComponent>();
			if (!TargetAttr || TargetAttr->IsDead()) continue;

			DamagedActors.Add(Actor);

			if (TargetAttr->ApplyCombatDamage(DamageBundle, CasterActor))
			{
				if (BurstHitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, BurstHitSound, Actor->GetActorLocation());
				}
				if (BurstHitVFX)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						GetWorld(), BurstHitVFX, Actor->GetActorLocation(),
						FRotator::ZeroRotator, FVector(1.0f), true, true, ENCPoolMethod::None, true);
				}
			}
		}
	}

	if (CurrentRadius >= MaxRadius)
	{
		Destroy();
	}
}
