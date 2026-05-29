#pragma once

#include "CoreMinimal.h"
#include "OffHandBase.h"
#include "Sound/SoundBase.h"
#include "NiagaraSystem.h"
#include "ShieldBase.generated.h"

UCLASS(Abstract)
class LOWPOLY_API AShieldBase : public AOffHandBase
{
	GENERATED_BODY()

public:
    AShieldBase();

    virtual void UseOffHand(class APlayerCharacter* Character) override;
    virtual void StopUseOffHand(class APlayerCharacter* Character) override;

    // --- Rarity-Aware Overrides ---
    virtual float GetPhysicalNegationForRarity(EItemRarity TargetRarity) const override;
    virtual float GetMagicNegationForRarity(EItemRarity TargetRarity) const override;
    virtual float GetFireNegationForRarity(EItemRarity TargetRarity) const override;
    virtual float GetLightningNegationForRarity(EItemRarity TargetRarity) const override;
    virtual float GetFrostNegationForRarity(EItemRarity TargetRarity) const override;
    virtual float GetPoisonNegationForRarity(EItemRarity TargetRarity) const override;
    virtual float GetHolyNegationForRarity(EItemRarity TargetRarity) const override;
    virtual float GetEarthNegationForRarity(EItemRarity TargetRarity) const override;

    // Legacy getters (linked to rarity logic)
    float GetPhysicalNegation() const { return GetCurrentPhysicalNegation(); }
    float GetMagicNegation() const { return GetCurrentMagicNegation(); }
    float GetFireNegation() const { return GetFireNegationForRarity(GetRarity()); }
    float GetLightningNegation() const { return GetLightningNegationForRarity(GetRarity()); }
    float GetFrostNegation() const { return GetFrostNegationForRarity(GetRarity()); }
    float GetPoisonNegation() const { return GetPoisonNegationForRarity(GetRarity()); }
    float GetHolyNegation() const { return GetHolyNegationForRarity(GetRarity()); }
    float GetEarthNegation() const { return GetEarthNegationForRarity(GetRarity()); }
    float GetStaminaCostOnBlock() const { return StaminaCostOnBlock; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Negation")
    float PhysicalNegation = 0.8f; // 0.0 to 1.0 (fallback if no rarity stats)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Negation")
    float MagicNegation = 0.2f; // 0.0 to 1.0 (fallback if no rarity stats)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Negation")
    float FireNegation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Negation")
    float LightningNegation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Negation")
    float FrostNegation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Negation")
    float PoisonNegation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Negation")
    float HolyNegation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Negation")
    float EarthNegation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    float StaminaCostOnBlock = 10.0f;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    class UAnimMontage* BlockMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    USoundBase* BlockSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    UNiagaraSystem* BlockEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    float BlockEffectScale = 1.0f;

public:
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnBlocked();
};
