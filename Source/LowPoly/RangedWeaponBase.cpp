#include "RangedWeaponBase.h"
#include "ProjectileBase.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

ARangedWeaponBase::ARangedWeaponBase()
{
	ItemType = EItemType::Ranged;
	bTwoHanded = true; // override to false in pistol BPs
	CurrentAmmo = MagazineCapacity;
    bUseCrosshair = true;

	PIPCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("PIPCapture"));
	PIPCapture->SetupAttachment(RootComponent);
	PIPCapture->bCaptureEveryFrame = false;
	PIPCapture->bCaptureOnMovement = false;
	PIPCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	PIPCapture->FOVAngle = PIPScopeFOV;
	PIPCapture->ShowFlags.SetBloom(false);
	PIPCapture->ShowFlags.SetVolumetricFog(false);
	PIPCapture->ShowFlags.SetMotionBlur(false);
	PIPCapture->SetActive(false);
}

void ARangedWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo = MagazineCapacity;

		// Attach PIP capture to the weapon's scope socket and apply settings.
	if (PIPCapture)
	{
		if (Mesh && ScopeSocketName != NAME_None && Mesh->DoesSocketExist(ScopeSocketName))
		{
			PIPCapture->AttachToComponent(
				Mesh,
				FAttachmentTransformRules::SnapToTargetIncludingScale,
				ScopeSocketName);
		}
		else
		{
			PIPCapture->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}

		PIPCapture->SetRelativeLocation(FVector::ZeroVector);
		PIPCapture->SetRelativeRotation(FRotator::ZeroRotator);
		PIPCapture->FOVAngle = PIPScopeFOV;
		PIPCapture->TextureTarget = PIPRenderTarget;
		PIPCapture->bCaptureEveryFrame = false;
		PIPCapture->bCaptureOnMovement = false;
		PIPCapture->SetActive(false);
	}
}

void ARangedWeaponBase::SetPIPCaptureEnabled(bool bEnabled)
{
	if (!PIPCapture)
	{
		return;
	}

	APlayerCharacter* OwningPlayer = Cast<APlayerCharacter>(GetOwner());
	if (!OwningPlayer)
	{
		OwningPlayer = Cast<APlayerCharacter>(GetAttachParentActor());
	}
	const bool bCanCaptureForLocalView = OwningPlayer && OwningPlayer->IsLocallyControlled();

	if (!bEnabled || !bUsePIPScope || !PIPRenderTarget || !bCanCaptureForLocalView)
	{
		PIPCapture->bCaptureEveryFrame = false;
		PIPCapture->bCaptureOnMovement = false;
		PIPCapture->SetActive(false);
		return;
	}

	PIPCapture->TextureTarget = PIPRenderTarget;
	PIPCapture->FOVAngle = PIPScopeFOV;
	PIPCapture->HiddenActors.Reset();
	PIPCapture->HideActorComponents(this);

	if (AActor* AttachOwner = GetAttachParentActor())
	{
		PIPCapture->HideActorComponents(AttachOwner);
	}

	PIPCapture->SetActive(true);
	PIPCapture->bCaptureEveryFrame = true;
	PIPCapture->bCaptureOnMovement = true;
	PIPCapture->CaptureScene();
}

void ARangedWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARangedWeaponBase, CurrentAmmo);
	DOREPLIFETIME(ARangedWeaponBase, bIsReloading);
	DOREPLIFETIME(ARangedWeaponBase, ReloadStartTime);
	DOREPLIFETIME(ARangedWeaponBase, bIsCyclingBolt);
	DOREPLIFETIME(ARangedWeaponBase, BoltCycleStartTime);
}

float ARangedWeaponBase::GetEffectiveShotsPerSecond() const
{
	if (FireRateRPM > 0.0f)
	{
		return FireRateRPM / 60.0f;
	}

	return FireRate;
}

bool ARangedWeaponBase::CanFire() const
{
	if (bIsReloading || bIsCyclingBolt || CurrentAmmo <= 0) return false;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float FireInterval = 1.0f / FMath::Max(GetEffectiveShotsPerSecond(), 0.01f);
	return (Now - LastFireTime) + 0.002f >= FireInterval;
}

void ARangedWeaponBase::Fire(AActor* ShooterActor)
{
	if (bBurstFire)
	{
		StartBurstFire(ShooterActor);
		return;
	}

	FireSingleShot(ShooterActor);
}

bool ARangedWeaponBase::FireSingleShot(AActor* ShooterActor)
{
	APlayerCharacter* Shooter = Cast<APlayerCharacter>(ShooterActor);
	if (!HasAuthority() || !CanFire() || !ProjectileClass || !Shooter) return false;

	FVector MuzzleLocation = GetActorLocation();
	if (UStaticMeshComponent* SM = FindComponentByClass<UStaticMeshComponent>())
	{
		if (SM->DoesSocketExist(MuzzleSocketName))
			MuzzleLocation = SM->GetSocketLocation(MuzzleSocketName);
	}

	FRotator FireRotation = Shooter->GetActorRotation();
	FVector TraceStart = MuzzleLocation;
	FVector TraceEnd = MuzzleLocation + FireRotation.Vector() * AimTraceDistance;
	FHitResult AimHit;
	FCollisionQueryParams AimParams;
	AimParams.AddIgnoredActor(Shooter);
	AimParams.AddIgnoredActor(this);

	if (AController* C = Shooter->GetController())
	{
		FVector CamLoc;
		FRotator CamRot;
		C->GetPlayerViewPoint(CamLoc, CamRot);

		TraceStart = CamLoc;
		TraceEnd = CamLoc + CamRot.Vector() * AimTraceDistance;

		const FVector TargetPoint = GetWorld()->LineTraceSingleByChannel(AimHit, TraceStart, TraceEnd, ECC_Visibility, AimParams)
			? AimHit.ImpactPoint
			: TraceEnd;

		FireRotation = (TargetPoint - MuzzleLocation).Rotation();
	}

	UAttributeComponent* Attr = Shooter->GetAttributeComponent();
	FDamageBundle ScaledDamageBundle = GetScaledDamage(Attr);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Shooter;
	SpawnParams.Instigator = Shooter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const int32 ProjectilesToSpawn = bUseShotgunPattern ? FMath::Max(1, PelletsPerShot) : 1;
	const float SpreadHalfAngleRad = FMath::DegreesToRadians(FMath::Max(0.0f, ShotgunSpreadHalfAngleDegrees));
	const float DamageScale = bUseShotgunPattern ? FMath::Max(0.0f, ShotgunPelletDamageScale) : 1.0f;

	for (int32 ProjectileIndex = 0; ProjectileIndex < ProjectilesToSpawn; ++ProjectileIndex)
	{
		FRotator SpawnRotation = FireRotation;
		if (bUseShotgunPattern && ProjectilesToSpawn > 1 && SpreadHalfAngleRad > 0.0f)
		{
			SpawnRotation = FMath::VRandCone(FireRotation.Vector(), SpreadHalfAngleRad).Rotation();
		}

		AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(
			ProjectileClass, MuzzleLocation, SpawnRotation, SpawnParams);
		if (Projectile)
		{
			FDamageBundle ProjectileDamage = ScaledDamageBundle;
			ProjectileDamage.Scale(DamageScale);
			Projectile->InitProjectile(Shooter, ProjectileDamage, this);
		}
	}

	CurrentAmmo--;
	LastFireTime = GetWorld()->GetTimeSeconds();

	if (FireMontage)
		Shooter->Multicast_PlayAttackMontage(FireMontage, NAME_None);

	Multicast_OnFired();

	const float YawKick = FMath::FRandRange(RecoilYawMin, RecoilYawMax);
	Shooter->Client_ApplyRecoil(RecoilPitch, YawKick);

	if (bScoped && Shooter->IsScoping() && bForceUnscopeAfterShot)
	{
		ScheduleForceUnscope(Shooter, BoltDescopeDelay);
	}

	if (CurrentAmmo <= 0)
		StartReload(Shooter);
	else if (bBoltAction)
		StartBoltCycle(Shooter);

	return true;
}

void ARangedWeaponBase::StartBurstFire(AActor* ShooterActor)
{
	APlayerCharacter* Shooter = Cast<APlayerCharacter>(ShooterActor);
	if (!HasAuthority() || !Shooter || bIsBursting) return;

	BurstFireShooter = Shooter;
	BurstShotsRemaining = FMath::Max(1, BurstShotsPerTrigger);
	bIsBursting = true;

	const bool bFired = FireSingleShot(BurstFireShooter);
	if (!bFired)
	{
		StopBurstFire();
		return;
	}

	BurstShotsRemaining--;
	if (BurstShotsRemaining <= 0 || bIsReloading || bIsCyclingBolt || CurrentAmmo <= 0)
	{
		StopBurstFire();
		return;
	}

	const float Interval = 1.0f / FMath::Max(GetEffectiveShotsPerSecond(), 0.01f);
	GetWorldTimerManager().SetTimer(BurstFireTimerHandle, this, &ARangedWeaponBase::BurstFireTick, Interval, false);
}

void ARangedWeaponBase::BurstFireTick()
{
	if (!bIsBursting || !BurstFireShooter)
	{
		StopBurstFire();
		return;
	}

	if (BurstShotsRemaining <= 0 || bIsReloading || bIsCyclingBolt || CurrentAmmo <= 0)
	{
		StopBurstFire();
		return;
	}

	if (!CanFire())
	{
		const float Interval = 1.0f / FMath::Max(GetEffectiveShotsPerSecond(), 0.01f);
		const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		const float NextAllowedFireTime = LastFireTime + Interval;
		const float RemainingDelay = FMath::Max(NextAllowedFireTime - Now, 0.001f);
		GetWorldTimerManager().SetTimer(BurstFireTimerHandle, this, &ARangedWeaponBase::BurstFireTick, RemainingDelay, false);
		return;
	}

	const bool bFired = FireSingleShot(BurstFireShooter);
	if (!bFired)
	{
		StopBurstFire();
		return;
	}

	BurstShotsRemaining--;
	if (BurstShotsRemaining <= 0 || bIsReloading || bIsCyclingBolt || CurrentAmmo <= 0)
	{
		StopBurstFire();
		return;
	}

	const float Interval = 1.0f / FMath::Max(GetEffectiveShotsPerSecond(), 0.01f);
	GetWorldTimerManager().SetTimer(BurstFireTimerHandle, this, &ARangedWeaponBase::BurstFireTick, Interval, false);
}

void ARangedWeaponBase::StopBurstFire()
{
	GetWorldTimerManager().ClearTimer(BurstFireTimerHandle);
	bIsBursting = false;
	BurstShotsRemaining = 0;
	BurstFireShooter = nullptr;
}

float ARangedWeaponBase::GetBoltProgress() const
{
	if (!bIsCyclingBolt) return 1.0f;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	return FMath::Clamp((Now - BoltCycleStartTime) / FMath::Max(BoltActionTime, 0.01f), 0.0f, 1.0f);
}

void ARangedWeaponBase::StartBoltCycle(AActor* ShooterActor)
{
    APlayerCharacter* Shooter = Cast<APlayerCharacter>(ShooterActor);
	if (!HasAuthority() || bIsCyclingBolt) return;

	bIsCyclingBolt = true;
	BoltCycleStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	if (Shooter && Shooter->IsAiming() && bForceUnscopeAfterShot)
	{
		ScheduleForceUnscope(Shooter, BoltDescopeDelay);
	}

	if (BoltActionMontage && Shooter)
		Shooter->Multicast_PlayAttackMontage(BoltActionMontage, NAME_None);

	Multicast_OnBoltCycleStarted();

	GetWorldTimerManager().SetTimer(
		BoltCycleTimerHandle,
		this,
		&ARangedWeaponBase::CompleteBoltCycle,
		BoltActionTime,
		false
	);
}

void ARangedWeaponBase::ScheduleForceUnscope(AActor* ShooterActor, float Delay)
{
    APlayerCharacter* Shooter = Cast<APlayerCharacter>(ShooterActor);
	if (!HasAuthority() || !Shooter)
	{
		return;
	}

	PendingDescopeShooter = Shooter;
	GetWorldTimerManager().ClearTimer(BoltDescopeTimerHandle);

	if (Delay > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			BoltDescopeTimerHandle,
			this,
			&ARangedWeaponBase::DoBoltDescope,
			Delay,
			false);
	}
	else
	{
		DoBoltDescope();
	}
}

void ARangedWeaponBase::CompleteBoltCycle()
{
	bIsCyclingBolt = false;
}

void ARangedWeaponBase::DoBoltDescope()
{
	if (PendingDescopeShooter && PendingDescopeShooter->IsAiming())
		PendingDescopeShooter->Server_SetAiming(false);
	PendingDescopeShooter = nullptr;
}

void ARangedWeaponBase::Multicast_OnBoltCycleStarted_Implementation()
{
	if (BoltActionSound)
		UGameplayStatics::PlaySoundAtLocation(this, BoltActionSound, GetActorLocation());
}

float ARangedWeaponBase::GetReloadProgress() const
{
	if (!bIsReloading) return 1.0f;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	return FMath::Clamp((Now - ReloadStartTime) / FMath::Max(ReloadTime, 0.01f), 0.0f, 1.0f);
}

float ARangedWeaponBase::GetCrosshairBloomAlpha() const
{
	const float Interval = 1.0f / FMath::Max(GetEffectiveShotsPerSecond(), 0.01f);
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float Elapsed = Now - LastFireTime;
	const float CooldownAlpha = FMath::Clamp(Elapsed / FMath::Max(Interval, 0.01f), 0.0f, 1.0f);
	return 1.0f - CooldownAlpha;
}

void ARangedWeaponBase::StartReload(AActor* ShooterActor)
{
    APlayerCharacter* Shooter = Cast<APlayerCharacter>(ShooterActor);
	if (!HasAuthority() || bIsReloading || CurrentAmmo >= MagazineCapacity) return;

	GetWorldTimerManager().ClearTimer(BoltCycleTimerHandle);
	bIsCyclingBolt = false;
	StopBurstFire();

	bIsReloading = true;
	ReloadStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	if (ReloadMontage && Shooter)
		Shooter->Multicast_PlayAttackMontage(ReloadMontage, NAME_None);

	Multicast_OnReloadStarted();

	GetWorldTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&ARangedWeaponBase::CompleteReload,
		ReloadTime,
		false
	);
}

void ARangedWeaponBase::CompleteReload()
{
	CurrentAmmo = MagazineCapacity;
	bIsReloading = false;
}

void ARangedWeaponBase::StartAutoFire(AActor* ShooterActor)
{
    APlayerCharacter* Shooter = Cast<APlayerCharacter>(ShooterActor);
	if (!HasAuthority() || !Shooter) return;

	AutoFireShooter = Shooter;
	Fire(Shooter);

	const float ShotInterval = 1.0f / FMath::Max(GetEffectiveShotsPerSecond(), 0.01f);
	const float TriggerInterval = bBurstFire ? ShotInterval * FMath::Max(1, BurstShotsPerTrigger) : ShotInterval;
	GetWorldTimerManager().SetTimer(
		AutoFireTimerHandle,
		this,
		&ARangedWeaponBase::AutoFireTick,
		TriggerInterval,
		true
	);
}

void ARangedWeaponBase::StopAutoFire()
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);
	AutoFireShooter = nullptr;
}

void ARangedWeaponBase::AutoFireTick()
{
	if (!AutoFireShooter)
	{
		StopAutoFire();
		return;
	}
	Fire(AutoFireShooter);
}

EWeaponHoldStance ARangedWeaponBase::GetHoldStance_Implementation() const
{
	return bTwoHanded ? EWeaponHoldStance::TwoHandedRanged : EWeaponHoldStance::OneHandedRanged;
}

FSkillSlotInfo ARangedWeaponBase::GetLMBSkillInfo() const
{
	FSkillSlotInfo Info;
	Info.bIsValid = true;
	Info.Icon = PrimaryActionIcon;
	Info.DisplayName = PrimaryActionName;
	const float EffectiveSPS = GetEffectiveShotsPerSecond();
	Info.bHasCooldown = EffectiveSPS > 0.0f;

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float Cooldown = (EffectiveSPS > 0.0f) ? 1.0f / EffectiveSPS : 0.0f;
	Info.CooldownProgress = (Cooldown > 0.0f)
		? FMath::Clamp((Now - LastFireTime) / Cooldown, 0.0f, 1.0f)
		: 1.0f;
	return Info;
}

FSkillSlotInfo ARangedWeaponBase::GetRMBSkillInfo() const
{
	FSkillSlotInfo Info;
	Info.bIsValid = true;
	Info.Icon = AimActionIcon ? AimActionIcon : PrimaryActionIcon;
	Info.DisplayName = AimActionName;
	Info.bHasCooldown = false;
	Info.CooldownProgress = 1.0f;
	return Info;
}

void ARangedWeaponBase::Multicast_OnFired_Implementation()
{
	if (FireSound)
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());

	if (MuzzleFlash)
	{
		FVector MuzzleLocation = GetActorLocation();
		FRotator MuzzleRotation = GetActorRotation();

		if (UStaticMeshComponent* SM = FindComponentByClass<UStaticMeshComponent>())
		{
			if (SM->DoesSocketExist(MuzzleSocketName))
			{
				MuzzleLocation = SM->GetSocketLocation(MuzzleSocketName);
				MuzzleRotation = SM->GetSocketRotation(MuzzleSocketName);
			}
		}

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, MuzzleFlash, MuzzleLocation, MuzzleRotation,
			FVector(MuzzleFlashScale), true, true, ENCPoolMethod::None, true);
	}

	BP_OnFired();
}

void ARangedWeaponBase::Multicast_OnReloadStarted_Implementation()
{
	if (ReloadSound)
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
}
