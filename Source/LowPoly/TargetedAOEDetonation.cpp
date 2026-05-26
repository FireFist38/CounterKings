#include "TargetedAOEDetonation.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AttributeComponent.h"
#include "PlayerCharacter.h"
#include "Net/UnrealNetwork.h"

ATargetedAOEDetonation::ATargetedAOEDetonation()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    DetonationVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DetonationVFX"));
    RootComponent = DetonationVFXComponent;
    DetonationVFXComponent->SetAutoActivate(false);
}

void ATargetedAOEDetonation::BeginPlay()
{
    Super::BeginPlay();
}

void ATargetedAOEDetonation::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATargetedAOEDetonation::InitDetonation(AActor* InCaster, const FDamageBundle& InDamage)
{
    if (!HasAuthority()) return;

    CasterActor = InCaster;
    DamageBundle = InDamage;

    ApplyDamage();
    Multicast_PlayDetonationVFX();

    // Destroy after lifetime
    SetLifeSpan(DetonationLifetime);
}

void ATargetedAOEDetonation::ApplyDamage()
{
    if (!HasAuthority()) return;

    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(DamageRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(CasterActor);
    Params.AddIgnoredActor(this);

    if (GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity, ECC_Pawn, Sphere, Params))
    {
        TArray<AActor*> DamagedActors;
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* Victim = Overlap.GetActor();
            if (Victim && !DamagedActors.Contains(Victim))
            {
                UAttributeComponent* Attr = Victim->FindComponentByClass<UAttributeComponent>();
                if (Attr)
                {
                    Attr->ApplyCombatDamage(DamageBundle, CasterActor);
                    DamagedActors.Add(Victim);
                }
            }
        }
    }

    if (bDebugDamage)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 12, FColor::Red, false, 2.0f);
    }
}

void ATargetedAOEDetonation::Multicast_PlayDetonationVFX_Implementation()
{
    if (DetonationSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DetonationSound, GetActorLocation());
    }

    if (DetonationHitVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, DetonationHitVFX, GetActorLocation());
    }
    
    if (DetonationVFXComponent)
    {
        DetonationVFXComponent->Activate();
    }
}

void ATargetedAOEDetonation::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}
