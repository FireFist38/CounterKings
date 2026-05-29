#pragma once

#include "CoreMinimal.h"
#include "MainHandBase.h"
#include "Sound/SoundBase.h"
#include "NiagaraSystem.h"

class APlayerCharacter; // Forward declaration

#include "RangedWeaponBase.generated.h"

class AProjectileBase;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

UCLASS(Abstract)
class LOWPOLY_API ARangedWeaponBase : public AMainHandBase
{
	GENERATED_BODY()

public:
	ARangedWeaponBase();

	virtual bool IsRanged() const override { return true; }

	virtual FSkillSlotInfo GetLMBSkillInfo() const override;
	virtual FSkillSlotInfo GetRMBSkillInfo() const override;

	virtual EWeaponHoldStance GetHoldStance_Implementation() const override;

	// Optional separate icon for the Aim (RMB) action; falls back to PrimaryActionIcon if null
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|HUD")
	UTexture2D* AimActionIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|HUD")
	FText AimActionName = NSLOCTEXT("Skill", "Aim", "Aim");

	bool CanFire() const;
	virtual void Fire(AActor* Shooter);
	void StartReload(AActor* Shooter);

	virtual void StartAutoFire(AActor* Shooter);
	virtual void StopAutoFire();

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	bool IsAutomatic() const { return bAutomatic; }

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	bool IsBurstFireEnabled() const { return bBurstFire; }

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	int32 GetBurstShotsPerTrigger() const { return BurstShotsPerTrigger; }

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	bool IsShotgunPatternEnabled() const { return bUseShotgunPattern; }

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	int32 GetPelletsPerShot() const { return PelletsPerShot; }

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	float GetFireRateRPM() const { return FireRateRPM; }

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	float GetEffectiveShotsPerSecond() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Scope")
	bool IsScoped() const { return bScoped; }

	UFUNCTION(BlueprintCallable, Category = "CK|Scope")
	float GetScopedFOV() const { return ScopedFOV; }

	UFUNCTION(BlueprintCallable, Category = "CK|Scope")
	FVector GetScopedCameraOffset() const { return ScopedCameraOffset; }

	UFUNCTION(BlueprintCallable, Category = "CK|Scope")
	float GetScopedArmLength() const { return ScopedArmLength; }

	UFUNCTION(BlueprintCallable, Category = "CK|Scope")
	FName GetScopeSocketName() const { return ScopeSocketName; }

	UFUNCTION(BlueprintCallable, Category = "CK|Scope")
	TSubclassOf<class UCKScopeOverlayWidget> GetScopeOverlayClass() const { return ScopeOverlayClass; }

	void SetPIPCaptureEnabled(bool bEnabled);

	USceneCaptureComponent2D* GetPIPCapture() const { return PIPCapture; }

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	virtual int32 GetMagazineCapacity() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	virtual float GetReloadTime() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	virtual float GetRecoilPitch() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	bool IsReloading() const { return bIsReloading; }

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	float GetReloadProgress() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	float GetCrosshairBloomAlpha() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	UAnimMontage* GetFireMontage() const { return FireMontage; }

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	UAnimMontage* GetReloadMontage() const { return ReloadMontage; }

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	UAnimMontage* GetAimMontage() const { return AimMontage; }

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnFired();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnReloadStarted();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnBoltCycleStarted();

	UFUNCTION(BlueprintCallable, Category = "CK|BoltAction")
	bool IsCyclingBolt() const { return bIsCyclingBolt; }

	UFUNCTION(BlueprintCallable, Category = "CK|BoltAction")
	float GetBoltProgress() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "CK|Ranged")
	void BP_OnFired();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Ranged")
	TSubclassOf<AProjectileBase> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged")
	FName MuzzleSocketName = FName("MuzzleSocket");

	// Legacy shots-per-second fallback used when FireRateRPM <= 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged")
	float FireRate = 1.0f;

	// Real-world style tuning value. If > 0, this overrides FireRate.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float FireRateRPM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged")
	int32 MagazineCapacity = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged")
	bool bAutomatic = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged|Burst")
	bool bBurstFire = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged|Burst", meta = (ClampMin = "1", UIMin = "1"))
	int32 BurstShotsPerTrigger = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged|Shotgun")
	bool bUseShotgunPattern = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged|Shotgun", meta = (ClampMin = "1", UIMin = "1"))
	int32 PelletsPerShot = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged|Shotgun", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ShotgunSpreadHalfAngleDegrees = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged|Shotgun", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ShotgunPelletDamageScale = 0.15f;

	// --- Bolt action settings ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|BoltAction")
	bool bBoltAction = false;

	// Seconds the bolt cycle takes (player can't fire until done)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|BoltAction")
	float BoltActionTime = 1.2f;

	// Seconds after firing before the player is forced out of scope/aim
	// (lets them see their hit before the cycle starts)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|BoltAction")
	float BoltDescopeDelay = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|BoltAction")
	USoundBase* BoltActionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|BoltAction")
	UAnimMontage* BoltActionMontage;

	// --- Scoped weapon settings ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scope")
	bool bScoped = false;

	// If true, the player is forced out of scope/aim after firing (or during bolt cycle).
	// Useful for bolt-action snipers, but can be disabled for semi-auto marksman rifles.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scope")
	bool bForceUnscopeAfterShot = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scope")
	float ScopedFOV = 55.0f;

	// Camera socket offset when scoped (relative to spring arm root)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scope")
	FVector ScopedCameraOffset = FVector(20.f, 0.f, 65.f);

	// Spring arm length when scoped (small/zero pulls camera off the character so the head doesn't block view)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scope")
	float ScopedArmLength = 0.0f;

	// Socket on the weapon mesh where the camera attaches when scoped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scope")
	FName ScopeSocketName = FName("ScopeSocket");

	// Widget shown over the screen while scoped (scope vignette + reticle)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scope")
	TSubclassOf<class UCKScopeOverlayWidget> ScopeOverlayClass;

	// --- PIP (picture-in-picture) scope rendering ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scope|PIP")
	bool bUsePIPScope = false;

	// FOV of the secondary scope camera (lower = more zoom). 10° ≈ 8x, 6° ≈ 16x
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scope|PIP")
	float PIPScopeFOV = 7.5f;

	// Render target that the scope camera writes to. Assign in BP. Sample this in the scope overlay material.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scope|PIP")
	UTextureRenderTarget2D* PIPRenderTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Scope|PIP")
	USceneCaptureComponent2D* PIPCapture;

	// Degrees of pitch kick applied per shot (camera kicks up)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Recoil")
	float RecoilPitch = 1.5f;

	// Random yaw kick per shot — picks between these two values
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Recoil")
	float RecoilYawMin = -0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Recoil")
	float RecoilYawMax = 0.4f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Ranged")
	int32 CurrentAmmo = 10;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Ranged")
	bool bIsReloading = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Ranged")
	float ReloadStartTime = 0.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|BoltAction")
	bool bIsCyclingBolt = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|BoltAction")
	float BoltCycleStartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged")
	float ReloadTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged")
	UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged")
	UAnimMontage* AimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged")
	USoundBase* ReloadSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged")
	UNiagaraSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged")
	float MuzzleFlashScale = 0.5f;

	// Max distance the aim trace checks for a target before defaulting to a far point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ranged")
	float AimTraceDistance = 10000.0f;

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Subclass Hooks ---
    float LastFireTime = -999.0f;
	FTimerHandle ReloadTimerHandle;
	FTimerHandle AutoFireTimerHandle;
	FTimerHandle BoltCycleTimerHandle;
	FTimerHandle BoltDescopeTimerHandle;
	FTimerHandle BurstFireTimerHandle;

	UPROPERTY()
	AActor* AutoFireShooter = nullptr;

	UPROPERTY()
	AActor* BurstFireShooter = nullptr;

	bool bIsBursting = false;
	int32 BurstShotsRemaining = 0;

	UPROPERTY()
	class APlayerCharacter* PendingDescopeShooter = nullptr;

    virtual void AutoFireTick();
	virtual bool FireSingleShot(AActor* Shooter);

private:
	void CompleteReload();
	void CompleteBoltCycle();
	void StartBoltCycle(AActor* Shooter);
	void ScheduleForceUnscope(AActor* Shooter, float Delay);
	void DoBoltDescope();
	void StartBurstFire(AActor* Shooter);
	void BurstFireTick();
	void StopBurstFire();
};
