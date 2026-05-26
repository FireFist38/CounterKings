#include "CKGameState.h"
#include "Net/UnrealNetwork.h"

ACKGameState::ACKGameState()
{
	CurrentRound = 0;
	MatchPhase = ECKMatchPhase::Waiting;
	PhaseEndServerTime = 0.0f;
	bDebugWin = false;
	WinStreak = 0;
	LossStreak = 0;
}

UDataTable* ACKGameState::GetLootDataTable() const
{
    return (ItemTables.Num() > 0) ? ItemTables[0] : nullptr;
}

void ACKGameState::SetSharedLootData(UDataTable* NewLootDataTable)
{
    if (NewLootDataTable)
    {
        ItemTables.Remove(NewLootDataTable);
        ItemTables.Insert(NewLootDataTable, 0);
    }
}

void ACKGameState::AddItemTable(UDataTable* NewTable)
{
	if (NewTable && !ItemTables.Contains(NewTable))
	{
		ItemTables.Add(NewTable);
	}
}

void ACKGameState::SetRoundState(int32 NewRound, ECKMatchPhase NewPhase, float DurationSeconds)
{
	CurrentRound = NewRound;
	MatchPhase = NewPhase;
	PhaseEndServerTime = GetServerWorldTimeSeconds() + FMath::Max(0.0f, DurationSeconds);
}

float ACKGameState::GetRemainingTime() const
{
	if (PhaseEndServerTime <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, PhaseEndServerTime - GetServerWorldTimeSeconds());
}

void ACKGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACKGameState, CurrentRound);
	DOREPLIFETIME(ACKGameState, MatchPhase);
	DOREPLIFETIME(ACKGameState, PhaseEndServerTime);
	DOREPLIFETIME(ACKGameState, ItemTables);
	DOREPLIFETIME(ACKGameState, bDebugWin);
	DOREPLIFETIME(ACKGameState, WinStreak);
	DOREPLIFETIME(ACKGameState, LossStreak);
}
