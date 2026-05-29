#pragma once

#include "CoreMinimal.h"
#include "OffHandBase.h"
#include "NiagaraSystem.h"
#include "ItemBase.h"
#include "SecondaryRangedBase.generated.h"

UCLASS(Abstract)
class LOWPOLY_API ASecondaryRangedBase : public AOffHandBase
{
	GENERATED_BODY()

public:
	ASecondaryRangedBase();

	virtual void UseOffHand(class APlayerCharacter* Character) override;
	virtual void StopUseOffHand(class APlayerCharacter* Character) override;

	virtual EWeaponHoldStance GetHoldStance_Implementation() const override;

	void StartReload(class APlayerCharacter* Shooter);
	void CompleteReload();
	float GetReloadProgress() const;
	bool IsReloading() const { return bIsReloading; }
	int32 GetCurrentAmmo() const { return CurrentAmmo; }
	int32 GetMagazineCapacity() const { return MagazineCapacity; }
	float GetCrosshairBloomAlpha() const;

	void Fire(class APlayerCharacter* Shooter);
	bool CanFire() const;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayMuzzleFlash();

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	bool IsShotgunPatternEnabled() const { return bUseShotgunPattern; }

	UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
	int32 GetPelletsPerShot() const { return PelletsPerShot; }

    UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
    bool IsAutomatic() const { return bAutomatic; }

    UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
    float GetFireRateRPM() const { return FireRateRPM; }

    // Souls-like letter scaling getters (used by UI; fields are protected)
    UFUNCTION(BlueprintCallable, Category = "CK|Scaling")
    EStatLetterScaling GetStrengthScalingLetter() const { return StrengthScalingLetter; }

    UFUNCTION(BlueprintCallable, Category = "CK|Scaling")
    EStatLetterScaling GetDexterityScalingLetter() const { return DexterityScalingLetter; }

    UFUNCTION(BlueprintCallable, Category = "CK|Scaling")
    EStatLetterScaling GetMagicScalingLetter() const { return MagicScalingLetter; }

    UFUNCTION(BlueprintCallable, Category = "CK|Scaling")
    EStatLetterScaling GetLuckScalingLetter() const { return LuckScalingLetter; }

    UFUNCTION(BlueprintCallable, Category = "CK|Ranged")
    float GetEffectiveShotsPerSecond() const;

    void StartAutoFire(class APlayerCharacter* Shooter);
    void StopAutoFire();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "CK|Combat")
	TSubclassOf<class AProjectileBase> ProjectileClass;

	// Legacy shots-per-second fallback used when FireRateRPM <= 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float FireRate = 2.0f;

    // Real-world style tuning value. If > 0, this overrides FireRate.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float FireRateRPM = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    bool bAutomatic = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float AimTraceDistance = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	int32 MagazineCapacity = 6;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	int32 CurrentAmmo = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float ReloadTime = 2.0f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CK|Combat")
	bool bIsReloading = false;

	// --- Shotgun settings ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Shotgun")
	bool bUseShotgunPattern = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Shotgun", meta = (ClampMin = "1", UIMin = "1"))
	int32 PelletsPerShot = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Shotgun", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ShotgunSpreadHalfAngleDegrees = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Shotgun", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ShotgunPelletDamageScale = 0.15f;

	// Souls-like letter scaling (preferred). When set to anything other than Unspecified,
	// the legacy float scaling values are ignored.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scaling")
	EStatLetterScaling StrengthScalingLetter = EStatLetterScaling::Unspecified;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scaling")
	EStatLetterScaling DexterityScalingLetter = EStatLetterScaling::Unspecified;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scaling")
	EStatLetterScaling MagicScalingLetter = EStatLetterScaling::Unspecified;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scaling")
	EStatLetterScaling LuckScalingLetter = EStatLetterScaling::Unspecified;

	// Legacy numeric scaling (kept for backward compatibility with existing blueprints).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scaling", meta = (DeprecatedProperty, EditCondition = "StrengthScalingLetter==EStatLetterScaling::Unspecified"))
	float StrengthScaling = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scaling", meta = (DeprecatedProperty, EditCondition = "DexterityScalingLetter==EStatLetterScaling::Unspecified"))
	float DexterityScaling = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scaling", meta = (DeprecatedProperty, EditCondition = "MagicScalingLetter==EStatLetterScaling::Unspecified"))
	float MagicScaling = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scaling", meta = (DeprecatedProperty, EditCondition = "LuckScalingLetter==EStatLetterScaling::Unspecified"))
	float LuckScaling = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	class UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	class UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	USoundBase* ReloadSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	class UNiagaraSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float MuzzleFlashScale = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float RecoilPitch = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float RecoilYawMin = -0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float RecoilYawMax = 0.2f;

    /** Returns the fully scaled damage bundle (internal; expose via public wrapper for UI) */
    FDamageBundle GetScaledDamage(const class UAttributeComponent* Attributes) const;

public:
    // UI helper: exposes the protected/scoped damage scaling to widgets safely.
    UFUNCTION(BlueprintCallable, Category = "CK|Combat")
    FDamageBundle GetScaledDamageForUI(const class UAttributeComponent* Attributes) const
    {
        return GetScaledDamage(Attributes);
    }

	float ComputeAttributeMultiplier(const class UAttributeComponent* Attributes) const;

protected:
	virtual void BeginPlay() override;

private:
	float LastFireTime = 0.0f;
	float ReloadStartTime = 0.0f;
	FTimerHandle ReloadTimerHandle;
	FTimerHandle AutoFireTimerHandle;

    UPROPERTY()
    class APlayerCharacter* AutoFireShooter = nullptr;

    void AutoFireTick();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
