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

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Negation")
    float PhysicalNegation = 0.8f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|Negation")
    float MagicNegation = 0.2f; // 0.0 to 1.0

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

public:
    float GetPhysicalNegation() const { return PhysicalNegation; }
    float GetMagicNegation() const { return MagicNegation; }
    float GetFireNegation() const { return FireNegation; }
    float GetLightningNegation() const { return LightningNegation; }
    float GetFrostNegation() const { return FrostNegation; }
    float GetPoisonNegation() const { return PoisonNegation; }
    float GetHolyNegation() const { return HolyNegation; }
    float GetEarthNegation() const { return EarthNegation; }
    float GetStaminaCostOnBlock() const { return StaminaCostOnBlock; }

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
