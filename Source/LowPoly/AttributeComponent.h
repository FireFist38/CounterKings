#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemBase.h"
#include "AttributeComponent.generated.h"

UENUM(BlueprintType)
enum class EAttributeType : uint8
{
	Health,
	Strength,
	Dexterity,
	Magic,
	Luck
};

class AFloatingDamageActor;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LOWPOLY_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();

protected:
	virtual void BeginPlay() override;

	// --- GDD Section 4: Attributes ---
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth, EditAnywhere, BlueprintReadWrite, Category = "CK|Attributes")
	float MaxHealth;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Attributes")
	float CurrentHealth;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Attributes")
	bool bIsDead;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Attributes")
	float MaxStamina;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Attributes")
	float CurrentStamina;

	// --- Stamina Settings ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Stamina")
	float StaminaDrainRate = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Stamina")
	float StaminaRegenRate = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Stamina")
	float RegenDelay = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Stamina")
	float ExhaustionDuration = 3.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Stamina")
	bool bIsExhausted;

	bool bIsSprinting;
	float LastStaminaChangeTime;
	float ExhaustionEndTime;

	// --- Mana ---
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Mana")
	float MaxMana = 100.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Mana")
	float CurrentMana = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Mana")
	float ManaRegenRate = 8.0f;

	// --- Progression ---
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Progression")
	int32 Level = 1;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Progression")
	float CurrentXP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Progression")
	float XPPerLevel = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Combat|FloatingDamage")
	TSubclassOf<AFloatingDamageActor> FloatingDamageActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|FloatingDamage")
	float FloatingDamageSpawnZOffset = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat|FloatingDamage")
	float FloatingDamageSpawnJitterXY = 24.0f;

public:
	UFUNCTION(BlueprintCallable, Category = "CK|Stamina")
	void SetSprinting(bool bNewSprinting);

    UFUNCTION(BlueprintCallable, Category = "CK|Stamina")
    bool ConsumeStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "CK|Stamina")
	void RestoreStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "CK|Stamina")
	bool IsSprinting() const { return bIsSprinting; }

	UFUNCTION(BlueprintCallable, Category = "CK|Stamina")
	bool CanSprint() const { return !bIsExhausted && CurrentStamina > 0; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Attributes")
	float Strength;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Attributes")
	float Dexterity;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Attributes")
	float Magic;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Attributes")
	float Luck;

	UPROPERTY(ReplicatedUsing = OnRep_AttributePoints, EditAnywhere, BlueprintReadWrite, Category = "CK|Progression")
	int32 AttributePoints;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Economy")
	int32 Gold;

	UFUNCTION()
	void OnRep_MaxHealth();

	UFUNCTION()
	void OnRep_AttributePoints();

public:	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** --- Perk modifier helpers (server-authoritative) --- */
	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	void ApplyStrengthBonus(float Delta);

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	void RemoveStrengthBonus(float Delta);

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	void ApplyDexterityBonus(float Delta);

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	void RemoveDexterityBonus(float Delta);

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	void ApplyMagicBonus(float Delta);

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	void RemoveMagicBonus(float Delta);

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	void ApplyLuckBonus(float Delta);

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	void RemoveLuckBonus(float Delta);

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	void ApplyHealthBonus(float Delta);

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	void RemoveHealthBonus(float Delta);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Progression")
	void Server_PurchaseXP();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Progression")
	void Server_UpgradeAttribute(EAttributeType Attribute);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Progression")
	void Server_CommitAttributeUpgrades(int32 HealthPoints, int32 StrengthPoints, int32 DexterityPoints, int32 MagicPoints, int32 LuckPoints);

	UFUNCTION(BlueprintCallable, Category = "CK|Economy")
	void AddGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "CK|Progression")
	void AddAttributePoints(int32 Points);

	UFUNCTION(BlueprintCallable, Category = "CK|Progression")
	void AddXP(float Amount);

	UFUNCTION(BlueprintCallable, Category = "CK|Progression")
	void SetLevel(int32 NewLevel);

	UFUNCTION(BlueprintCallable, Category = "CK|Progression")
	void SetCurrentXP(float NewXP);

	UFUNCTION(BlueprintCallable, Category = "CK|Combat")
	bool ApplyCombatDamage(const FDamageBundle& Damage, AActor* DamageInstigator);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnFloatingDamage(float DamageAmount, EDamageType DamageType);

	UFUNCTION(BlueprintCallable, Category = "CK|Combat")
	void RestoreHealth(float HealAmount);

	// Getters
	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintCallable, Category = "CK|Attributes")
	int32 GetGold() const { return Gold; }
	
	UFUNCTION(BlueprintCallable, Category = "CK|Progression")
	int32 GetAttributePoints() const { return AttributePoints; }

	UFUNCTION(BlueprintCallable, Category = "CK|Progression")
	int32 GetLevel() const { return Level; }

	UFUNCTION(BlueprintCallable, Category = "CK|Progression")
	float GetCurrentXP() const { return CurrentXP; }

	UFUNCTION(BlueprintCallable, Category = "CK|Progression")
	float GetXPThreshold() const { return Level * XPPerLevel; }

	UFUNCTION(BlueprintCallable, Category = "CK|Mana")
	float GetCurrentMana() const { return CurrentMana; }

	UFUNCTION(BlueprintCallable, Category = "CK|Mana")
	float GetMaxMana() const { return MaxMana; }

	UFUNCTION(BlueprintCallable, Category = "CK|Mana")
	bool ConsumeMana(float Amount);

	UFUNCTION(BlueprintCallable, Category = "CK|Mana")
	void RestoreMana(float Amount);
};
