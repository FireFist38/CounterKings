#include "TargetedAOEIndicator.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ATargetedAOEIndicator::ATargetedAOEIndicator()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    IndicatorVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("IndicatorVFX"));
    RootComponent = IndicatorVFXComponent;
    IndicatorVFXComponent->SetAutoActivate(false);
}

void ATargetedAOEIndicator::BeginPlay()
{
    Super::BeginPlay();
}

void ATargetedAOEIndicator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Track ground point locally for smoothness, and on server for authority
    APlayerCharacter* Caster = Cast<APlayerCharacter>(GetOwner());
    if (Caster && (HasAuthority() || Caster->IsLocallyControlled()))
    {
        FVector GroundPoint;
        if (GetAimGroundPoint(GroundPoint))
        {
            SetActorLocation(GroundPoint);
        }
    }
}

void ATargetedAOEIndicator::InitIndicator(float InRadius, float InMaxRange)
{
    IndicatorRadius = InRadius;
    MaxTargetRange = InMaxRange;

    if (HasAuthority())
    {
        Multicast_ActivateVFX();
    }
}

void ATargetedAOEIndicator::Multicast_ActivateVFX_Implementation()
{
    if (IndicatorVFXComponent)
    {
        IndicatorVFXComponent->SetVariableFloat(FName("Radius"), IndicatorRadius);
        IndicatorVFXComponent->Activate();
    }
}

bool ATargetedAOEIndicator::GetAimGroundPoint(FVector& OutPoint) const
{
    APlayerCharacter* Caster = Cast<APlayerCharacter>(GetOwner());
    if (!Caster) return false;

    UCameraComponent* Camera = Caster->GetFollowCamera();
    if (!Camera) return false;

    FVector TraceStart = Camera->GetComponentLocation();
    FVector TraceEnd = TraceStart + (Camera->GetForwardVector() * MaxTargetRange);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Caster);
    Params.AddIgnoredActor(this);
    
    // Explicitly ignore all pawns to prevent the indicator from "climbing" characters/dummies
    TArray<AActor*> FoundPawns;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundPawns);
    Params.AddIgnoredActors(FoundPawns);

    // Trace specifically for the ground. 
    // ECC_WorldStatic is usually floors/walls, but we use a list of object types to be safer.
    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);

    if (GetWorld()->LineTraceSingleByObjectType(Hit, TraceStart, TraceEnd, ObjectParams, Params))
    {
        OutPoint = Hit.Location;
        return true;
    }

    // Fallback: If no hit, project onto a horizontal plane at caster's feet
    float DistanceToFloor = 200.0f; // Typical height from floor
    FVector FloorPoint = Caster->GetActorLocation() - FVector(0,0, DistanceToFloor);
    FPlane FloorPlane(FloorPoint, FVector::UpVector);
    
    FVector IntersectionPoint;
    if (FMath::SegmentPlaneIntersection(TraceStart, TraceEnd, FloorPlane, IntersectionPoint))
    {
        OutPoint = IntersectionPoint;
        return true;
    }

    return false;
}
