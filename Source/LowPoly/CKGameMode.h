#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CKGameMode.generated.h"

UCLASS()
class LOWPOLY_API ACKGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACKGameMode();

	UFUNCTION(BlueprintCallable, Category = "CK|GameLoop")
	void StartRound();

	UFUNCTION(BlueprintCallable, Category = "CK|GameLoop")
	void StartPostRoundPhase();

	UFUNCTION(BlueprintCallable, Category = "CK|GameLoop")
	void StartPostRoundPhaseWithResult(bool bWin);

	void AdvanceRound();

    /** Called by a timer after the result screen has played, then travels to the lobby map */
    void DelayedTravelToLobby();

    // --- Match Damage Scaling ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Rules")
    int32 BaseMatchDamage = 5;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Rules")
    int32 RoundDamageScaling = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Loot")
	class UDataTable* LootDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Loot")
	TArray<class UDataTable*> AdditionalItemTables;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Rules")
	int32 MaxRounds = 40;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Rules")
	float PostRoundDuration = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Rules")
	float RoundDuration = 180.0f;

    // --- Map Travel ---
    /** The lobby map to travel to after each round */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Lobby")
    FSoftObjectPath LobbyMapPath;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Lobby")
    FName ArenaSpawnTag = FName("ArenaSpawn");

protected:
	int32 CurrentRound = 0;

	FTimerHandle RoundTimerHandle;

    /** Separate timer for the round/phase duration (doesn't get overridden by DelayedTravelToLobby) */
    FTimerHandle RoundPhaseTimerHandle;

	virtual void BeginPlay() override;

	void HandleArenaTransition();
	void SpawnLootInArena();
	void CleanupArenaLoot();

    /** Saves all player data into PlayerState, then travels to the lobby map via ServerTravel */
    void TravelToLobby();

    /** Teleports players to arena spawn points */
    void TeleportPlayersToArena();

    /** Restores player data on newly spawned characters after map travel */
    virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

public:
	UFUNCTION(BlueprintCallable, Category = "CK|GameLoop")
	int32 GetCurrentRound() const { return CurrentRound; }

	UFUNCTION(BlueprintCallable, Category = "CK|GameLoop")
	float GetRemainingTime() const;
};
