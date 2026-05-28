#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ItemBase.h"
#include "AbilityBase.h"
#include "PerkBase.h"
#include "CKGameState.generated.h"

UENUM(BlueprintType)
enum class ECKMatchPhase : uint8
{
	Waiting,
	Combat,
	PostRound,
	GameOver
};

UCLASS()
class LOWPOLY_API ACKGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ACKGameState();

	UFUNCTION(BlueprintCallable, Category = "CK|GameLoop")
	int32 GetCurrentRound() const { return CurrentRound; }

	UFUNCTION(BlueprintCallable, Category = "CK|GameLoop")
	ECKMatchPhase GetMatchPhase() const { return MatchPhase; }

	UFUNCTION(BlueprintCallable, Category = "CK|GameLoop")
	float GetRemainingTime() const;

	UFUNCTION(BlueprintCallable, Category = "CK|Loot")
	const TArray<class UDataTable*>& GetAllItemTables() const { return ItemTables; }

    // Compatibility shim: returns first table
    UFUNCTION(BlueprintCallable, Category = "CK|Loot")
    class UDataTable* GetLootDataTable() const;

	void AddItemTable(class UDataTable* NewTable);

    // Compatibility shim: clears and adds one table
    void SetSharedLootData(class UDataTable* NewLootDataTable);

	void SetRoundState(int32 NewRound, ECKMatchPhase NewPhase, float DurationSeconds);

	// --- Streaks & Results ---
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Economy")
	int32 WinStreak;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Economy")
	int32 LossStreak;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|GameLoop")
	bool bDebugWin;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Player Persistence (persists across ServerTravel) ---
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
	int32 SavedMatchHealth = 100;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<FName> SavedShopPool;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<bool> SavedLockedSlots;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<TSubclassOf<AItemBase>> SavedMainHandClasses;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<int32> SavedMainHandRarities;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<int32> SavedMainHandGoldValues;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<TSubclassOf<AItemBase>> SavedOffHandClasses;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<int32> SavedOffHandRarities;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<int32> SavedOffHandGoldValues;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<TSubclassOf<AItemBase>> SavedInventoryClasses;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<int32> SavedInventoryRarities;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<int32> SavedInventoryGoldValues;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<TSubclassOf<AAbilityBase>> SavedAbilityClasses;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<int32> SavedAbilityRarities;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<int32> SavedAbilityGoldValues;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<TSubclassOf<APerkBase>> SavedPerkClasses;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<int32> SavedPerkRarities;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TArray<int32> SavedPerkGoldValues;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TSubclassOf<AItemBase> SavedArmorClass;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	int32 SavedArmorRarity = 0;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	int32 SavedArmorGoldValue = 0;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	TSubclassOf<AItemBase> SavedConsumableClass;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	int32 SavedConsumableRarity = 0;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
	int32 SavedConsumableGoldValue = 0;

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|GameLoop")
	int32 CurrentRound;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|GameLoop")
	ECKMatchPhase MatchPhase;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|GameLoop")
	float PhaseEndServerTime;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Loot")
	TArray<class UDataTable*> ItemTables;
};
