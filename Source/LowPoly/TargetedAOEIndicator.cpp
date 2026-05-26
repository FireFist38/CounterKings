#include "TargetedAOEIndicator.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
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

    // Trace against WorldStatic and WorldDynamic (ground/floors)
    if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
    {
        OutPoint = Hit.Location;
        return true;
    }

    // Fallback: If no hit, project onto a horizontal plane at caster's feet
    float DistanceToFloor = 200.0f; // Typical height from floor
    FVector FloorPoint = Caster->GetActorLocation() - FVector(0,0, DistanceToFloor);
    FPlane FloorPlane(FloorPoint, FVector::UpVector);
    
    float T;
    FVector Intersection;
    if (FMath::SegmentPlaneIntersection(TraceStart, TraceEnd, FloorPlane, T, Intersection))
    {
        OutPoint = Intersection;
        return true;
    }

    return false;
}
