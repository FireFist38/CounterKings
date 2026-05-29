#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CKInteractable.h"
#include "ItemBase.generated.h"

struct FLootTableEntry;

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Common,
	Uncommon,
	Rare,
	Legendary
};

UENUM(BlueprintType)
enum class EStatLetterScaling : uint8
{
	Unspecified UMETA(DisplayName = "Unspecified"),
	E UMETA(DisplayName = "E"),
	D UMETA(DisplayName = "D"),
	C UMETA(DisplayName = "C"),
	B UMETA(DisplayName = "B"),
	A UMETA(DisplayName = "A"),
	S UMETA(DisplayName = "S")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Melee,
	Ranged,
	Magic,
	Armor,
	Consumable,
	Ability,
	Perk
};

UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Physical,
	Magic,
	Fire,
	Lightning,
	Frost,
	Poison,
	Holy,
	Earth
};

UENUM(BlueprintType)
enum class ECrosshairStyle : uint8
{
	Default,
	Shotgun,
	Bow,
	Precision,
	Magic,
	Offhand,
	Unique
};

USTRUCT(BlueprintType)
struct FDamageBundle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float Physical = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float Magic = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float Fire = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float Lightning = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float Frost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float Poison = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float Holy = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	float Earth = 0.0f;

	FDamageBundle() {}

	float GetTotal() const
	{
		return Physical + Magic + Fire + Lightning + Frost + Poison + Holy + Earth;
	}

	void Scale(float Multiplier)
	{
		Physical *= Multiplier;
		Magic *= Multiplier;
		Fire *= Multiplier;
		Lightning *= Multiplier;
		Frost *= Multiplier;
		Poison *= Multiplier;
		Holy *= Multiplier;
		Earth *= Multiplier;
	}
};

/**
 * Per-rarity stats for items (weapons, shields, tomes, armor).
 * Allows designers to set explicit values for each rarity tier in the Blueprint editor.
 */
USTRUCT(BlueprintType)
struct FRarityStats
{
	GENERATED_BODY()

	// Damage values for weapons (melee, ranged, magic, offhand ranged)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity")
	FDamageBundle Damage;

	// Shield negation values (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PhysicalNegation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MagicNegation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FireNegation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LightningNegation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FrostNegation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PoisonNegation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HolyNegation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EarthNegation = 0.0f;

	// Armor stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity")
	float HealthBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity")
	float MovementSpeedModifier = 1.0f;

	// --- Ranged Weapon Stats ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity")
	int32 MagazineCapacity = 0; // If > 0, overrides base MagazineCapacity

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity")
	float FireRateRPM = 0.0f; // If > 0, overrides base FireRateRPM

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity")
	float ReloadTime = 0.0f; // If > 0, overrides base ReloadTime

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity")
	float RecoilPitch = 0.0f; // If > 0, overrides base RecoilPitch

	// --- Melee/Magic Resource Stats ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity")
	float ManaCost = 0.0f; // If > 0, overrides base ManaCost

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity")
	float StaminaCost = 0.0f; // If > 0, overrides base StaminaCost

	// Gold value override (optional; if 0, uses base GoldValue * rarity scale)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity")
	int32 GoldValueOverride = 0;

	FRarityStats()
	{
	}
};

UCLASS(Abstract, BlueprintType, Blueprintable)
class LOWPOLY_API AItemBase : public AActor, public ICKInteractable
{
	GENERATED_BODY()
	
public:	
	AItemBase();

protected:
	virtual void BeginPlay() override;

    // --- ICKInteractable Interface ---
    virtual void Interact_Implementation(APlayerCharacter* Interactor) override;
    virtual FText GetInteractionText_Implementation() const override;

	void RestoreAuthoredMeshRelativeTransform();

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Item")
	EItemType ItemType;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Item")
	FName ItemName;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Item")
    FText Description;

	UPROPERTY(ReplicatedUsing = OnRep_Rarity, EditAnywhere, BlueprintReadWrite, Category = "CK|Item")
	EItemRarity Rarity;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Item")
	int32 GoldValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Components")
	class USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Components")
	class UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Components")
	class USphereComponent* InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Components")
	class UPointLightComponent* RarityLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Item")
	TMap<EItemRarity, FLinearColor> RarityColors;

	UFUNCTION()
	void OnRep_Rarity();

	void UpdateRarityVisuals();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|HUD")
    bool bUseCrosshair = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|HUD")
    ECrosshairStyle CrosshairStyle = ECrosshairStyle::Default;

public:	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Per-Rarity Stats System ---
	
	/**
	 * Map of rarity to stats. Designers fill this in the Blueprint editor.
	 * If a rarity entry exists, those stats are used. Otherwise, falls back to base stats.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Rarity")
	TMap<EItemRarity, FRarityStats> RarityStats;

	/**
	 * Returns true if this item has per-rarity stats defined (not perks/abilities).
	 * Used to determine whether to use rarity stats or legacy scaling.
	 */
	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	bool HasRarityStats() const { return RarityStats.Num() > 0; }

	/**
	 * Get the rarity stats for a specific rarity tier.
	 * @param TargetRarity The rarity to look up
	 * @param OutStats Output struct with the stats
	 * @return True if stats exist for this rarity, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	bool GetRarityStatsFor(EItemRarity TargetRarity, FRarityStats& OutStats) const;

	/**
	 * Get the damage bundle for a specific rarity (for weapons).
	 * Returns the rarity-specific damage if defined, otherwise returns base damage.
	 */
	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	FDamageBundle GetDamageForRarity(EItemRarity TargetRarity) const;

	/**
	 * Get the current damage based on the item's current rarity.
	 */
	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	FDamageBundle GetCurrentDamage() const { return GetDamageForRarity(Rarity); }

	// --- Negation Getters ---

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetPhysicalNegationForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetMagicNegationForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetFireNegationForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetLightningNegationForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetFrostNegationForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetPoisonNegationForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetHolyNegationForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetEarthNegationForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	float GetCurrentPhysicalNegation() const { return GetPhysicalNegationForRarity(Rarity); }

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	float GetCurrentMagicNegation() const { return GetMagicNegationForRarity(Rarity); }

	// --- Armor Getters ---

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetHealthBonusForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetMovementSpeedModifierForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	float GetCurrentHealthBonus() const { return GetHealthBonusForRarity(Rarity); }

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	float GetCurrentMovementSpeedModifier() const { return GetMovementSpeedModifierForRarity(Rarity); }

	// --- Ranged Getters ---

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual int32 GetMagazineCapacityForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetFireRateRPMForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetReloadTimeForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetRecoilPitchForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	int32 GetCurrentMagazineCapacity() const { return GetMagazineCapacityForRarity(Rarity); }

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	float GetCurrentFireRateRPM() const { return GetFireRateRPMForRarity(Rarity); }

	// --- Resource Getters ---

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetManaCostForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	virtual float GetStaminaCostForRarity(EItemRarity TargetRarity) const;

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	float GetCurrentManaCost() const { return GetManaCostForRarity(Rarity); }

	UFUNCTION(BlueprintCallable, Category = "CK|Rarity")
	float GetCurrentStaminaCost() const { return GetStaminaCostForRarity(Rarity); }

	// --- Base Stats (fallback when no rarity stats defined) ---
	
	/**
	 * Base damage for this item. Used as fallback when no rarity stats are defined.
	 * Override in weapon subclasses.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
	FDamageBundle BaseDamage;

	// --- Standard Item Methods ---

	UFUNCTION(BlueprintCallable, Category = "CK|Item")
	void OnPickedUp();

	UFUNCTION(BlueprintCallable, Category = "CK|Item")
	void OnDropped(FVector DropLocation);

	void ApplyLootTableEntry(const FLootTableEntry& Entry);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetStoredState();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetDroppedState(FVector DropLocation);

	virtual void Equip(class USceneComponent* AttachParent, FName SocketName);
	virtual void Unequip();

	UFUNCTION(BlueprintCallable, Category = "CK|Item")
	FName GetItemName() const { return ItemName; }

    UFUNCTION(BlueprintCallable, Category = "CK|Item")
    FText GetDescription() const { return Description; }

    UFUNCTION(BlueprintCallable, Category = "CK|Item")
    void SetItemName(FName NewName) { ItemName = NewName; }

    UFUNCTION(BlueprintCallable, Category = "CK|Item")
    void SetDescription(FText NewDescription) { Description = NewDescription; }

    UFUNCTION(BlueprintCallable, Category = "CK|Item")
    void SetRarity(EItemRarity NewRarity) { Rarity = NewRarity; }

    UFUNCTION(BlueprintCallable, Category = "CK|Item")
    void SetGoldValue(int32 NewValue) { GoldValue = NewValue; }

	UFUNCTION(BlueprintCallable, Category = "CK|Item")
	EItemType GetItemType() const { return ItemType; }

	UFUNCTION(BlueprintCallable, Category = "CK|Item")
	EItemRarity GetRarity() const { return Rarity; }

	UFUNCTION(BlueprintCallable, Category = "CK|Item")
	int32 GetGoldValue() const { return GoldValue; }

	UFUNCTION(BlueprintCallable, Category = "CK|Item")
	EItemRarity GetRarityInternal() const { return Rarity; }

	UFUNCTION(BlueprintCallable, Category = "CK|Item")
	void SetRarityInternal(EItemRarity NewRarity) { Rarity = NewRarity; }

	UFUNCTION(BlueprintCallable, Category = "CK|Item")
	void SetGoldValueInternal(int32 NewValue) { GoldValue = NewValue; }

    static float GetRarityStrengthScale(EItemRarity InRarity)
    {
        switch (InRarity)
        {
        case EItemRarity::Common:    return 1.0f;
        case EItemRarity::Uncommon:  return 1.25f;
        case EItemRarity::Rare:      return 1.5f;
        case EItemRarity::Legendary: return 2.0f;
        default:                     return 1.0f;
        }
    }

    // Letter -> coefficient mapping for souls-like stat scaling.
    // These coefficients are intentionally small so +1 stat does not explode DPS.
    static float GetLetterStatScalingCoeff(EStatLetterScaling InLetter)
    {
        switch (InLetter)
        {
        case EStatLetterScaling::E:  return 0.05f;
        case EStatLetterScaling::D:  return 0.075f;
        case EStatLetterScaling::C:  return 0.10f;
        case EStatLetterScaling::B:  return 0.125f;
        case EStatLetterScaling::A:  return 0.15f;
        case EStatLetterScaling::S:  return 0.20f;
        case EStatLetterScaling::Unspecified:
        default:                     return 0.0f;
        }
    }

    UFUNCTION(BlueprintCallable, Category = "CK|HUD")
    bool ShouldShowCrosshair() const { return bUseCrosshair; }

    UFUNCTION(BlueprintCallable, Category = "CK|HUD")
    ECrosshairStyle GetCrosshairStyle() const { return CrosshairStyle; }
};
