#include "SecondaryRangedBase.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "ProjectileBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"



ASecondaryRangedBase::ASecondaryRangedBase()
{
	ActionName = NSLOCTEXT("Skill", "Shoot", "Shoot");
    BaseDamage.Physical = 10.0f;
	CurrentAmmo = MagazineCapacity;
}

void ASecondaryRangedBase::BeginPlay()
{
	Super::BeginPlay();

	// Ensure we start with a full magazine based on the potentially overridden capacity
	CurrentAmmo = MagazineCapacity;
}

void ASecondaryRangedBase::UseOffHand(APlayerCharacter* Character)
{
	if (bAutomatic)
    {
        StartAutoFire(Character);
    }
    else if (CanFire())
	{
		Fire(Character);
	}
}

void ASecondaryRangedBase::StopUseOffHand(APlayerCharacter* Character)
{
    StopAutoFire();
}

EWeaponHoldStance ASecondaryRangedBase::GetHoldStance_Implementation() const
{
	return EWeaponHoldStance::OneHandedMelee; // Placeholder for secondary ranged pose
}

void ASecondaryRangedBase::StartReload(APlayerCharacter* Shooter)
{
	if (bIsReloading || CurrentAmmo >= MagazineCapacity) return;

    StopAutoFire();

	bIsReloading = true;
	ReloadStartTime = GetWorld()->GetTimeSeconds();

	if (ReloadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
	}

	if (ReloadMontage && Shooter)
	{
		Shooter->Multicast_PlayOffhandAttackMontage(ReloadMontage, NAME_None);
	}

	GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &ASecondaryRangedBase::CompleteReload, ReloadTime, false);
}

void ASecondaryRangedBase::CompleteReload()
{
	bIsReloading = false;
	CurrentAmmo = MagazineCapacity;
}

float ASecondaryRangedBase::GetReloadProgress() const
{
	if (!bIsReloading || ReloadTime <= 0.0f) return 1.0f;
	const float Elapsed = GetWorld()->GetTimeSeconds() - ReloadStartTime;
	return FMath::Clamp(Elapsed / ReloadTime, 0.0f, 1.0f);
}

void ASecondaryRangedBase::Fire(APlayerCharacter* Shooter)
{
	if (!HasAuthority() || !Shooter || !ProjectileClass) return;

	CurrentAmmo--;
	LastFireTime = GetWorld()->GetTimeSeconds();

	FVector MuzzleLocation = GetActorLocation();
	FRotator BaseSpawnRotation = Shooter->GetControlRotation();

	if (USkeletalMeshComponent* MeshComp = FindComponentByClass<USkeletalMeshComponent>())
	{
		MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
	}

	// Trace to find impact point for rotation
	if (APlayerController* PC = Cast<APlayerController>(Shooter->GetController()))
	{
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);
		
		FVector TraceEnd = CamLoc + (CamRot.Vector() * AimTraceDistance);
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Shooter);
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, TraceEnd, ECC_Visibility, Params))
		{
			BaseSpawnRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, Hit.ImpactPoint);
		}
		else
		{
			BaseSpawnRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, TraceEnd);
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Shooter;
	SpawnParams.Instigator = Shooter;

	const int32 ProjectilesToSpawn = bUseShotgunPattern ? FMath::Max(1, PelletsPerShot) : 1;
	const float SpreadHalfAngleRad = FMath::DegreesToRadians(FMath::Max(0.0f, ShotgunSpreadHalfAngleDegrees));
	const float DamageScale = bUseShotgunPattern ? FMath::Max(0.0f, ShotgunPelletDamageScale) : 1.0f;

	for (int32 i = 0; i < ProjectilesToSpawn; ++i)
	{
		FRotator SpawnRotation = BaseSpawnRotation;
		if (bUseShotgunPattern && ProjectilesToSpawn > 1 && SpreadHalfAngleRad > 0.0f)
		{
			SpawnRotation = FMath::VRandCone(BaseSpawnRotation.Vector(), SpreadHalfAngleRad).Rotation();
		}

		AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, MuzzleLocation, SpawnRotation, SpawnParams);
		if (Projectile)
		{
			FDamageBundle ProjectileDamage = GetScaledDamage(Shooter->GetAttributeComponent());
			ProjectileDamage.Scale(DamageScale);
			Projectile->InitProjectile(Shooter, ProjectileDamage, this);
		}
	}

	Multicast_PlayMuzzleFlash();
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	if (FireMontage)
	{
		Shooter->Multicast_PlayOffhandAttackMontage(FireMontage, NAME_None);
	}

    Shooter->Client_ApplyRecoil(RecoilPitch, FMath::FRandRange(RecoilYawMin, RecoilYawMax));

    if (CurrentAmmo <= 0)
    {
        StartReload(Shooter);
    }
}

float ASecondaryRangedBase::GetEffectiveShotsPerSecond() const
{
    if (FireRateRPM > 0.0f)
    {
        return FireRateRPM / 60.0f;
    }
    return FireRate;
}

bool ASecondaryRangedBase::CanFire() const
{
	if (bIsReloading || CurrentAmmo <= 0) return false;
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    const float FireInterval = 1.0f / FMath::Max(GetEffectiveShotsPerSecond(), 0.01f);
    return (Now - LastFireTime) + 0.002f >= FireInterval;
}

void ASecondaryRangedBase::StartAutoFire(APlayerCharacter* Shooter)
{
    if (!HasAuthority() || !Shooter) return;

    AutoFireShooter = Shooter;
    if (CanFire())
    {
        Fire(Shooter);
    }

    const float Interval = 1.0f / FMath::Max(GetEffectiveShotsPerSecond(), 0.01f);
    GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this, &ASecondaryRangedBase::AutoFireTick, Interval, true);
}

void ASecondaryRangedBase::StopAutoFire()
{
    if (!HasAuthority()) return;
    GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);
    AutoFireShooter = nullptr;
}

void ASecondaryRangedBase::AutoFireTick()
{
    if (!AutoFireShooter || !CanFire())
    {
        if (AutoFireShooter && CurrentAmmo <= 0 && !bIsReloading)
        {
            StartReload(AutoFireShooter);
        }
        return;
    }

    Fire(AutoFireShooter);
}

float ASecondaryRangedBase::GetCrosshairBloomAlpha() const
{
	const float Interval = 1.0f / FMath::Max(GetEffectiveShotsPerSecond(), 0.01f);
	const float Elapsed = GetWorld()->GetTimeSeconds() - LastFireTime;
	const float CooldownAlpha = FMath::Clamp(Elapsed / FMath::Max(Interval, 0.01f), 0.0f, 1.0f);
	return 1.0f - CooldownAlpha;
}

void ASecondaryRangedBase::Multicast_PlayMuzzleFlash_Implementation()
{
	if (MuzzleFlash)
	{
		// Spawn muzzle flash attached to mesh
	}
}

FDamageBundle ASecondaryRangedBase::GetScaledDamage(const UAttributeComponent* Attributes) const
{
    FDamageBundle Scaled = GetCurrentDamage();
    Scaled.Scale(ComputeAttributeMultiplier(Attributes));
    return Scaled;
}

float ASecondaryRangedBase::ComputeAttributeMultiplier(const UAttributeComponent* Attributes) const
{
	if (!Attributes) return 1.0f;

	const float RarityScale = AItemBase::GetRarityStrengthScale(Rarity);

	const bool bUseLetterScaling =
		StrengthScalingLetter != EStatLetterScaling::Unspecified ||
		DexterityScalingLetter != EStatLetterScaling::Unspecified ||
		MagicScalingLetter != EStatLetterScaling::Unspecified ||
		LuckScalingLetter != EStatLetterScaling::Unspecified;

	const float StrengthCoeff  = bUseLetterScaling ? AItemBase::GetLetterStatScalingCoeff(StrengthScalingLetter)  : StrengthScaling;
	const float DexterityCoeff = bUseLetterScaling ? AItemBase::GetLetterStatScalingCoeff(DexterityScalingLetter) : DexterityScaling;
	const float MagicCoeff     = bUseLetterScaling ? AItemBase::GetLetterStatScalingCoeff(MagicScalingLetter)     : MagicScaling;
	const float LuckCoeff      = bUseLetterScaling ? AItemBase::GetLetterStatScalingCoeff(LuckScalingLetter)      : LuckScaling;

	const float WeightedSum =
		  Attributes->Strength  * StrengthCoeff
		+ Attributes->Dexterity * DexterityCoeff
		+ Attributes->Magic     * MagicCoeff
		+ Attributes->Luck      * LuckCoeff;

	return 1.0f + RarityScale * WeightedSum;
}

void ASecondaryRangedBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASecondaryRangedBase, CurrentAmmo);
	DOREPLIFETIME(ASecondaryRangedBase, bIsReloading);
}

