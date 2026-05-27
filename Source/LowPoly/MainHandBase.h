#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "Components/BoxComponent.h"
#include "WeaponStanceTypes.h"
#include "SkillSlotInfo.h"

class APlayerCharacter; // Forward declaration
class UAnimMontage;
class UTexture2D;
class USoundBase;

#include "MainHandBase.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class LOWPOLY_API AMainHandBase : public AItemBase
{
	GENERATED_BODY()

public:
	AMainHandBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Components")
	class UBoxComponent* HitboxComponent;

    // --- Combat Stats ---
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    FDamageBundle BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    TArray<FName> ComboSectionNames;

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
    // If the corresponding Letter is Unspecified, we will use these values.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scaling", meta = (DeprecatedProperty, EditCondition = "StrengthScalingLetter==EStatLetterScaling::Unspecified"))
    float StrengthScaling = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scaling", meta = (DeprecatedProperty, EditCondition = "DexterityScalingLetter==EStatLetterScaling::Unspecified"))
    float DexterityScaling = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scaling", meta = (DeprecatedProperty, EditCondition = "MagicScalingLetter==EStatLetterScaling::Unspecified"))
    float MagicScaling = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Scaling", meta = (DeprecatedProperty, EditCondition = "LuckScalingLetter==EStatLetterScaling::Unspecified"))
    float LuckScaling = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    int32 MaxHitTargets = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    FVector HitboxExtent = FVector(50.0f, 50.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    bool bDrawDebugAttack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    bool bUseMana = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    float ManaCost = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    float StaminaCost = 0.0f;

    // --- Skill Slot Info ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    UTexture2D* PrimaryActionIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    FText PrimaryActionName;

    // --- Methods ---
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "CK|Combat")
    FDamageBundle GetScaledDamage(const UAttributeComponent* Attributes) const;

    UFUNCTION(BlueprintCallable, Category = "CK|Combat")
    FName GetComboSectionName(int32 ComboIndex) const;

    UFUNCTION(BlueprintCallable, Category = "CK|Combat")
    UAnimMontage* GetAttackMontage() const { return AttackMontage; }

	UFUNCTION(BlueprintCallable, Category = "CK|Combat")
	virtual void BeginMeleeHitWindow(AActor* AttackingCharacter);

	UFUNCTION(BlueprintCallable, Category = "CK|Combat")
	virtual void TickMeleeHitWindow(AActor* AttackingCharacter);

	UFUNCTION(BlueprintCallable, Category = "CK|Combat")
	virtual void EndMeleeHitWindow();

	UFUNCTION(BlueprintCallable, Category = "CK|Combat")
	virtual bool ApplyMeleeHit(AActor* AttackingCharacter);

    UFUNCTION(BlueprintCallable, Category = "CK|Combat")
    virtual float ComputeAttributeMultiplier(const UAttributeComponent* Attributes) const;

    UFUNCTION(BlueprintCallable, Category = "CK|Combat")
    virtual bool IsRanged() const;

    UFUNCTION(BlueprintCallable, Category = "CK|Combat")
    virtual bool IsMagic() const;

    UFUNCTION(BlueprintCallable, Category = "CK|Combat")
    virtual bool IsTwoHanded() const;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnMeleeHit();

    UFUNCTION(BlueprintImplementableEvent, Category = "CK|Combat")
    void BP_OnMeleeHit();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CK|Combat")
    EWeaponHoldStance GetHoldStance() const;

    virtual FSkillSlotInfo GetLMBSkillInfo() const;
    virtual FSkillSlotInfo GetRMBSkillInfo() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	bool bTwoHanded = false;

private:
	TSet<AActor*> HitActorsThisWindow;
	bool bHitWindowOpen = false;
};
