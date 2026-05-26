#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"

class APlayerCharacter; // Forward declaration

#include "AbilityBase.generated.h"

/**
 * Base class for active abilities. 
 * Abilities are cooldown-based and unlocked via Player Level.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class LOWPOLY_API AAbilityBase : public AItemBase
{
	GENERATED_BODY()

public:
	AAbilityBase();

	// --- Ability Settings ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ability")
	float Cooldown = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ability")
	UAnimMontage* ActivationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Ability")
	USoundBase* ActivationSound;

	// --- Execution ---
	UFUNCTION(BlueprintCallable, Category = "CK|Ability")
	virtual bool CanActivate(AActor* Character) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Ability")
	virtual void ActivateAbility(AActor* Character);

    UFUNCTION(BlueprintImplementableEvent, Category = "CK|Ability")
    void BP_OnActivated(AActor* Character);

protected:
    // Tracks the last time the ability was used for cooldown logic.
    // Replicated so clients can show accurate cooldown timers.
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Ability")
    float LastActivationTime = -999.0f;

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "CK|Ability")
    float GetCooldownRemaining() const;

    UFUNCTION(BlueprintCallable, Category = "CK|Ability")
    float GetCooldownNormalized() const;
};
