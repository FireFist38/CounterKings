#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
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
