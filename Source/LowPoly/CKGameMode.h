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

    // --- 3D Lobby Tags ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Lobby")
    FName LobbySpawnTag = FName("LobbySpawn");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Lobby")
    FName ArenaSpawnTag = FName("ArenaSpawn");

protected:
	int32 CurrentRound = 0;

	FTimerHandle RoundTimerHandle;

	virtual void BeginPlay() override;

	void HandleArenaTransition();
	void SpawnLootInArena();
	void CleanupArenaLoot();

    // Teleports all players to points with the given tag
    void TeleportAllPlayers(FName TargetTag);

    void TeleportPlayersToLobby();
    void TeleportPlayersToArena();

public:
	UFUNCTION(BlueprintCallable, Category = "CK|GameLoop")
	int32 GetCurrentRound() const { return CurrentRound; }

	UFUNCTION(BlueprintCallable, Category = "CK|GameLoop")
	float GetRemainingTime() const;
};
