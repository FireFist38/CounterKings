// Fill out your copyright notice in the Description page of Project Settings.

#include "CKGameMode.h"
#include "CKGameState.h"
#include "CKPlayerState.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "LootSpawner.h"
#include "ItemBase.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerStart.h"

ACKGameMode::ACKGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	GameStateClass = ACKGameState::StaticClass();
    PlayerStateClass = ACKPlayerState::StaticClass();

    static ConstructorHelpers::FObjectFinder<UDataTable> PerksTable(TEXT("/Game/Data/DT_Perks"));
    if (PerksTable.Succeeded())
    {
        AdditionalItemTables.Add(PerksTable.Object);
    }
    static ConstructorHelpers::FObjectFinder<UDataTable> WeaponsTable(TEXT("/Game/Data/DT_Weapons"));
    if (WeaponsTable.Succeeded())
    {
        AdditionalItemTables.Add(WeaponsTable.Object);
    }
    static ConstructorHelpers::FObjectFinder<UDataTable> ArmorTable(TEXT("/Game/Data/DT_Armor"));
    if (ArmorTable.Succeeded())
    {
        AdditionalItemTables.Add(ArmorTable.Object);
    }
    static ConstructorHelpers::FObjectFinder<UDataTable> AbilitiesTable(TEXT("/Game/Data/DT_Abilities"));
    if (AbilitiesTable.Succeeded())
    {
        AdditionalItemTables.Add(AbilitiesTable.Object);
    }
}

void ACKGameMode::BeginPlay()
{
	Super::BeginPlay();

    // Dynamically load tables at runtime to bypass Blueprint serialization issues
    TArray<FString> TablePaths = {
        TEXT("/Game/Data/DT_Perks"),
        TEXT("/Game/Data/DT_Weapons"),
        TEXT("/Game/Data/DT_Armor"),
        TEXT("/Game/Data/DT_Abilities")
    };

    for (const FString& Path : TablePaths)
    {
        UDataTable* LoadedTable = LoadObject<UDataTable>(nullptr, *Path);
        if (LoadedTable && !AdditionalItemTables.Contains(LoadedTable))
        {
            AdditionalItemTables.Add(LoadedTable);
        }
    }

	if (ACKGameState* CKGameState = GetGameState<ACKGameState>())
	{
		CKGameState->SetSharedLootData(LootDataTable);
        for (UDataTable* Table : AdditionalItemTables)
        {
            CKGameState->AddItemTable(Table);
        }
	}
	
	// Start the game loop
	StartRound();
}

void ACKGameMode::StartRound()
{
	if (!HasAuthority()) return;

	CurrentRound++;
	UE_LOG(LogTemp, Log, TEXT("Starting Round %d"), CurrentRound);

	if (ACKGameState* CKGameState = GetGameState<ACKGameState>())
	{
		CKGameState->SetRoundState(CurrentRound, ECKMatchPhase::Combat, RoundDuration);
	}

	// GDD Section 9: Arena changes every 5 rounds
	if (CurrentRound > 1 && CurrentRound % 5 == 1)
	{
		HandleArenaTransition();
	}

    TeleportPlayersToArena();

	CleanupArenaLoot();
	SpawnLootInArena();

	GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &ACKGameMode::AdvanceRound, RoundDuration, false);
}

void ACKGameMode::StartPostRoundPhase()
{
	StartPostRoundPhaseWithResult(false);
}

void ACKGameMode::StartPostRoundPhaseWithResult(bool bWin)
{
	if (!HasAuthority()) return;

	ACKGameState* GS = GetGameState<ACKGameState>();
	if (!GS) return;

	// Calculate and Grant Rewards for all players
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
        APlayerController* PC = It->Get();
		APlayerCharacter* Char = Cast<APlayerCharacter>(PC->GetPawn());
        ACKPlayerState* PS = Cast<ACKPlayerState>(PC->PlayerState);

		if (Char && Char->GetAttributeComponent() && PS)
		{
            // Apply Tournament Damage
            if (!bWin)
            {
                // Damage = (Base + Round Scaling) + (Loser's Missing HP)
                int32 BaseDamage = BaseMatchDamage + (CurrentRound * RoundDamageScaling);
                int32 MissingHealth = FMath::Max(0, Char->GetAttributeComponent()->GetMaxHealth() - Char->GetAttributeComponent()->GetCurrentHealth());
                
                PS->MatchHealth -= (BaseDamage + MissingHealth);
                
                if (PS->MatchHealth <= 0)
                {
                    PS->MatchHealth = 0;
                    UE_LOG(LogTemp, Warning, TEXT("Player Eliminated: %s"), *PC->GetName());
                }
            }

			// Math: Base Income
			int32 Payout = 100;

			// Math: Interest
			if (Char->GetAttributeComponent()->GetGold() >= 1000)
				Payout += 50;

			// Math: Streaks
			if (bWin)
			{
				GS->WinStreak++;
				GS->LossStreak = 0;
			}
			else
			{
				GS->LossStreak++;
				GS->WinStreak = 0;
			}

			int32 CurrentStreak = bWin ? GS->WinStreak : GS->LossStreak;
			if (CurrentStreak >= 3 && CurrentStreak <= 5) Payout += 20;
			else if (CurrentStreak >= 6 && CurrentStreak <= 8) Payout += 40;
			else if (CurrentStreak >= 9) Payout += 60;

			Char->GetAttributeComponent()->AddGold(Payout);
            
            // C++ Trigger for Shop Generation
            Char->GenerateShopPool();

            // Reset ready state for next round
            Char->Server_SetReadyForNextRound(false);
		}
	}

	GS->bDebugWin = bWin;
	GS->SetRoundState(CurrentRound, ECKMatchPhase::PostRound, PostRoundDuration);

    TeleportPlayersToLobby();

	GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &ACKGameMode::AdvanceRound, PostRoundDuration, false);
}

void ACKGameMode::AdvanceRound()
{
	if (CurrentRound >= MaxRounds)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game Over - Max Rounds Reached"));
		if (ACKGameState* CKGameState = GetGameState<ACKGameState>())
		{
			CKGameState->SetRoundState(CurrentRound, ECKMatchPhase::GameOver, 0.0f);
		}
		return;
	}

	StartRound();
}

void ACKGameMode::SpawnLootInArena()
{
	TArray<AActor*> Spawners;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALootSpawner::StaticClass(), Spawners);

	for (AActor* SpawnerActor : Spawners)
	{
		ALootSpawner* Spawner = Cast<ALootSpawner>(SpawnerActor);
		if (Spawner)
		{
			Spawner->SpawnLoot(CurrentRound, 1.0f);
		}
	}
}

void ACKGameMode::CleanupArenaLoot()
{
	TArray<AActor*> Items;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AItemBase::StaticClass(), Items);

	for (AActor* ItemActor : Items)
	{
		AItemBase* Item = Cast<AItemBase>(ItemActor);
		if (Item && !Item->IsHidden())
		{
			Item->Destroy();
		}
	}
}

void ACKGameMode::HandleArenaTransition()
{
	UE_LOG(LogTemp, Warning, TEXT("Arena Changing for Round %d"), CurrentRound);
}

void ACKGameMode::TeleportAllPlayers(FName TargetTag)
{
    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetTag, SpawnPoints);

    if (SpawnPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No spawn points found with tag: %s"), *TargetTag.ToString());
        return;
    }

    int32 SpawnIndex = 0;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->GetPawn())
        {
            AActor* TargetSpawn = SpawnPoints[SpawnIndex % SpawnPoints.Num()];
            PC->GetPawn()->SetActorLocationAndRotation(TargetSpawn->GetActorLocation(), TargetSpawn->GetActorRotation(), false, nullptr, ETeleportType::TeleportPhysics);
            SpawnIndex++;
        }
    }
}

void ACKGameMode::TeleportPlayersToLobby()
{
    TeleportAllPlayers(LobbySpawnTag);
}

void ACKGameMode::TeleportPlayersToArena()
{
    TeleportAllPlayers(ArenaSpawnTag);
}

float ACKGameMode::GetRemainingTime() const
{
	if (const ACKGameState* CKGameState = GetGameState<ACKGameState>())
	{
		return CKGameState->GetRemainingTime();
	}

	return 0.0f;
}
