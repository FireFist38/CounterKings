#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "ArmorBase.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class LOWPOLY_API AArmorBase : public AItemBase
{
	GENERATED_BODY()

public:
	AArmorBase();

    // --- Rarity-Aware Overrides ---
    virtual float GetHealthBonusForRarity(EItemRarity TargetRarity) const override;
    virtual float GetMovementSpeedModifierForRarity(EItemRarity TargetRarity) const override;

    // Legacy getters (linked to rarity logic)
    float GetHealthBonus() const { return GetCurrentHealthBonus(); }
    float GetMovementSpeedModifier() const { return GetCurrentMovementSpeedModifier(); }

protected:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Stats")
	float HealthBonus; // Fallback if no rarity stats

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Stats")
	float MovementSpeedModifier; // Fallback if no rarity stats

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
