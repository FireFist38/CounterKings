#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ItemBase.h"
#include "CKPlayerState.generated.h"

UCLASS()
class LOWPOLY_API ACKPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ACKPlayerState();

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Match")
	int32 MatchHealth = 100;

    // --- Persistent Data (survives ServerTravel) ---

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedLevel = 1;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    float SavedXP = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedGold = 0;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedAttributePoints = 0;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    float SavedStrength = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    float SavedDexterity = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    float SavedMagic = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    float SavedLuck = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<FName> SavedShopPool;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<bool> SavedLockedSlots;

    // --- Inventory Persistence (survives ServerTravel) ---
    // Main Hand Slots (2 slots)
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<TSubclassOf<AItemBase>> SavedMainHandClasses;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedMainHandRarities;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedMainHandGoldValues;

    // Off Hand Slots (2 slots)
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<TSubclassOf<AItemBase>> SavedOffHandClasses;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedOffHandRarities;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedOffHandGoldValues;

    // Bag Inventory Slots (12 slots)
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<TSubclassOf<AItemBase>> SavedInventoryClasses;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedInventoryRarities;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedInventoryGoldValues;

    // Ability Slots (3 slots)
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<TSubclassOf<AItemBase>> SavedAbilityClasses;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedAbilityRarities;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedAbilityGoldValues;

    // Perk Slots (10 slots)
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<TSubclassOf<AItemBase>> SavedPerkClasses;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedPerkRarities;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<int32> SavedPerkGoldValues;

    // Armor Slot (1 slot)
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TSubclassOf<AItemBase> SavedArmorClass;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedArmorRarity = 0;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedArmorGoldValue = 0;

    // Consumable Slot (1 slot)
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TSubclassOf<AItemBase> SavedConsumableClass;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedConsumableRarity = 0;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedConsumableGoldValue = 0;

    // Match Health (persists across rounds)
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedMatchHealth = 100;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};