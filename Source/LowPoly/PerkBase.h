#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"

class APlayerCharacter; // Forward declaration

#include "PerkBase.generated.h"

/**
 * Base class for passive perks.
 * Perks provide stat modifiers while equipped in a Perk slot.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class LOWPOLY_API APerkBase : public AItemBase
{
	GENERATED_BODY()

public:
	APerkBase();

	/** Called when the perk is moved into an active Perk slot */
	UFUNCTION(BlueprintCallable, Category = "CK|Perk")
	virtual void OnPerkActivated(AActor* Character);

	/** Called when the perk is moved out of an active Perk slot */
	UFUNCTION(BlueprintCallable, Category = "CK|Perk")
	virtual void OnPerkDeactivated(AActor* Character);

    /** Hook to modify damage received by the player holding this perk */
    UFUNCTION(BlueprintCallable, Category = "CK|Perk")
    virtual void ModifyIncomingDamage(UPARAM(ref) FDamageBundle& InOutDamage, AActor* DamageInstigator);

    /** Hook to modify damage dealt by the player holding this perk */
    UFUNCTION(BlueprintCallable, Category = "CK|Perk")
    virtual void ModifyOutgoingDamage(UPARAM(ref) FDamageBundle& InOutDamage, AActor* Target);

    // --- Damage Type Multipliers ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageBuff")
    float PhysicalDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageBuff")
    float MagicDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageBuff")
    float FireDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageBuff")
    float LightningDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageBuff")
    float FrostDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageBuff")
    float PoisonDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageBuff")
    float HolyDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageBuff")
    float EarthDamageMultiplier = 1.0f;

    // --- Attribute Bonuses ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|Attributes")
    float StrengthBonus = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|Attributes")
    float DexterityBonus = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|Attributes")
    float MagicBonus = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|Attributes")
    float LuckBonus = 0.0f;

    /** Adds to MaxHealth while perk is active (also clamps CurrentHealth). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|Attributes")
    float HealthBonus = 0.0f;

    // --- Damage Type Negation (Percentage: 0.0 = 0%, 1.0 = 100%) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageNegation")
    float PhysicalNegation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageNegation")
    float MagicNegation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageNegation")
    float FireNegation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageNegation")
    float LightningNegation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageNegation")
    float FrostNegation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageNegation")
    float PoisonNegation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageNegation")
    float HolyNegation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Perk|DamageNegation")
    float EarthNegation = 0.0f;

    UFUNCTION(BlueprintImplementableEvent, Category = "CK|Perk")
    void BP_OnPerkActivated(AActor* Character);

    UFUNCTION(BlueprintImplementableEvent, Category = "CK|Perk")
    void BP_OnPerkDeactivated(AActor* Character);

    UFUNCTION(BlueprintImplementableEvent, Category = "CK|Perk")
    void BP_ModifyIncomingDamage(UPARAM(ref) FDamageBundle& InOutDamage, AActor* DamageInstigator);

    UFUNCTION(BlueprintImplementableEvent, Category = "CK|Perk")
    void BP_ModifyOutgoingDamage(UPARAM(ref) FDamageBundle& InOutDamage, AActor* Target);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "CK|Perk")
    AActor* OwningActor;
};
