#include "RangedBeamWeaponBase.h"
#include "ABeamBase.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "Camera/CameraComponent.h"

ARangedBeamWeaponBase::ARangedBeamWeaponBase()
{
    bAutomatic = true;
    FireRateRPM = 600.0f; // 10 ammo per second
}

void ARangedBeamWeaponBase::StartAutoFire(AActor* ShooterActor)
{
    if (!HasAuthority() || !ShooterActor || ActiveBeam) return;

    APlayerCharacter* Shooter = Cast<APlayerCharacter>(ShooterActor);
    if (!Shooter || !BeamActorClass) return;

    FActorSpawnParameters Params;
    Params.Owner = Shooter;
    Params.Instigator = Shooter;

    ActiveBeam = GetWorld()->SpawnActor<ABeamBase>(BeamActorClass, GetActorLocation(), GetActorRotation(), Params);
    if (ActiveBeam)
    {
        if (UStaticMeshComponent* SM = FindComponentByClass<UStaticMeshComponent>())
        {
            ActiveBeam->InitBeamTracking(SM, MuzzleSocketName);
        }
    }

    // Play the Fire montage when the beam starts
    if (FireMontage)
    {
        Shooter->Multicast_PlayAttackMontage(FireMontage, NAME_None);
    }

    // Do NOT call Multicast_OnFired() here; ABeamBase handles the looping sound.

    Super::StartAutoFire(ShooterActor);
}

void ARangedBeamWeaponBase::StopAutoFire()
{
    if (HasAuthority())
    {
        if (ActiveBeam)
        {
            ActiveBeam->Destroy();
            ActiveBeam = nullptr;
        }

        if (AutoFireShooter)
        {
            if (APlayerCharacter* Shooter = Cast<APlayerCharacter>(AutoFireShooter))
            {
                if (FireMontage)
                {
                    Shooter->Multicast_StopMontage(FireMontage, 0.2f);
                }
            }
        }
    }

    Super::StopAutoFire();
}

void ARangedBeamWeaponBase::Fire(AActor* Shooter)
{
    if (!HasAuthority() || !Shooter) return;

    // Beam weapons handle ammo consumption per "Fire" tick from the auto-fire timer
    if (CurrentAmmo <= 0)
    {
        StopAutoFire();
        StartReload(Shooter);
        return;
    }

    CurrentAmmo--;
    LastFireTime = GetWorld()->GetTimeSeconds();

    UpdateBeamState(Shooter);
}

void ARangedBeamWeaponBase::AutoFireTick()
{
    if (!AutoFireShooter)
    {
        StopAutoFire();
        return;
    }

    Fire(AutoFireShooter);
}

bool ARangedBeamWeaponBase::FireSingleShot(AActor* Shooter)
{
    // Beam weapons are continuous; single shots aren't supported in this model
    return false;
}

void ARangedBeamWeaponBase::UpdateBeamState(AActor* ShooterActor)
{
    APlayerCharacter* Shooter = Cast<APlayerCharacter>(ShooterActor);
    if (!Shooter || !ActiveBeam) return;

    FVector TraceStart;
    FRotator TraceRotation;

    if (AController* C = Shooter->GetController())
    {
        FVector CamLoc;
        FRotator CamRot;
        C->GetPlayerViewPoint(CamLoc, CamRot);
        TraceStart = CamLoc;
        TraceRotation = CamRot;
    }
    else
    {
        TraceStart = GetActorLocation();
        TraceRotation = GetActorRotation();
    }

    const FVector TraceEnd = TraceStart + (TraceRotation.Vector() * BeamRange);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Shooter);
    Params.AddIgnoredActor(this);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

    // Muzzle location for the visual start
    FVector MuzzleLocation = GetActorLocation();
    if (UStaticMeshComponent* SM = FindComponentByClass<UStaticMeshComponent>())
    {
        if (SM->DoesSocketExist(MuzzleSocketName))
            MuzzleLocation = SM->GetSocketLocation(MuzzleSocketName);
    }

    const FVector EndPoint = bHit ? Hit.ImpactPoint : TraceEnd;
    ActiveBeam->UpdateBeam(MuzzleLocation, EndPoint, bHit, Hit);

    // Periodic Damage application
    const float Now = GetWorld()->GetTimeSeconds();
    if (bHit && (Now - LastDamageTime >= DamageTickInterval))
    {
        if (AActor* HitActor = Hit.GetActor())
        {
            if (UAttributeComponent* TargetAttr = HitActor->FindComponentByClass<UAttributeComponent>())
            {
                FDamageBundle ScaledDamage = GetScaledDamage(Shooter->GetAttributeComponent());
                TargetAttr->ApplyCombatDamage(ScaledDamage, Shooter);
                LastDamageTime = Now;
            }
        }
    }
}

void ARangedBeamWeaponBase::Multicast_OnFired_Implementation()
{
    // Empty implementation: Beam sounds are managed by ABeamBase.
}
