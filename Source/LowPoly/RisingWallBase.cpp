#include "RisingWallBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

ARisingWallBase::ARisingWallBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(Root);
	CollisionBox->SetBoxExtent(FVector(120.0f, 24.0f, 120.0f));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);

	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	WallMesh->SetupAttachment(CollisionBox);
	WallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ARisingWallBase::BeginPlay()
{
	Super::BeginPlay();

	TargetLocation = GetActorLocation();
	StartLocation = TargetLocation - FVector(0.0f, 0.0f, RiseHeight);
	SetActorLocation(StartLocation);

	if (HasAuthority() && Lifetime > 0.0f)
	{
		SetLifeSpan(Lifetime);
	}
}

void ARisingWallBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bHasReachedFinalHeight)
	{
		return;
	}

	RiseElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(RiseElapsed / FMath::Max(RiseDuration, 0.01f), 0.0f, 1.0f);
	const FVector NewLocation = FMath::Lerp(StartLocation, TargetLocation, Alpha);
	SetActorLocation(NewLocation);

	if (Alpha >= 1.0f)
	{
		bHasReachedFinalHeight = true;
	}
}

void ARisingWallBase::ConfigureWall(float InLifetime, float InRiseDuration, float InRiseHeight)
{
	Lifetime = InLifetime;
	RiseDuration = InRiseDuration;
	RiseHeight = InRiseHeight;
}
