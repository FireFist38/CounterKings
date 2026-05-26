#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

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

UCLASS(Abstract, BlueprintType, Blueprintable)
class LOWPOLY_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AItemBase();

protected:
	virtual void BeginPlay() override;

	void RestoreAuthoredMeshRelativeTransform();

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Item")
	EItemType ItemType;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Item")
	FName ItemName;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Item")
    FText Description;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Item")
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

	UFUNCTION(BlueprintCallable, Category = "CK|Item")
	void OnPickedUp();

	UFUNCTION(BlueprintCallable, Category = "CK|Item")
	void OnDropped(FVector DropLocation);

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
