#include "LootSpawner.h"
#include "LootGenerationData.h"
#include "Components/BillboardComponent.h"
#include "Engine/DataTable.h"

ALootSpawner::ALootSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	EditorVisualizer = CreateDefaultSubobject<UBillboardComponent>(TEXT("Visualizer"));
	RootComponent = EditorVisualizer;
}

void ALootSpawner::BeginPlay()
{
	Super::BeginPlay();
}

AItemBase* ALootSpawner::SpawnLoot(int32 CurrentRound, float PlayerLuck)
{
	if (!HasAuthority()) return nullptr;

    // --- DEBUG OVERRIDE ---
    if (DebugGuaranteedItem)
    {
        FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AItemBase* DebugItem = GetWorld()->SpawnActor<AItemBase>(DebugGuaranteedItem, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (DebugItem)
		{
			for (UDataTable* Table : LootTables)
			{
				if (!Table) continue;

				TArray<FLootTableEntry*> Rows;
				Table->GetAllRows<FLootTableEntry>(TEXT("DebugLootHydration"), Rows);
				for (const FLootTableEntry* Row : Rows)
				{
					if (Row && Row->ItemClass && Row->ItemClass == DebugGuaranteedItem)
					{
						DebugItem->ApplyLootTableEntry(*Row);
						return DebugItem;
					}
				}
			}
		}
		return DebugItem;
    }
    // ----------------------

	if (LootTables.Num() == 0 || !GenerationSettings) return nullptr;

	EItemRarity ChosenRarity = RollRarity(CurrentRound, PlayerLuck);

	// Collect ALL items from ALL tables
    TArray<FLootTableEntry*> AllPotentialItems;
    for (UDataTable* Table : LootTables)
    {
        if (Table)
        {
            Table->GetAllRows<FLootTableEntry>(TEXT("LootRolling"), AllPotentialItems);
        }
    }

	TArray<FLootTableEntry*> MatchingTierItems;
	float TotalWeight = 0.0f;

	for (auto* Entry : AllPotentialItems)
	{
		if (Entry->Rarity == ChosenRarity)
		{
			MatchingTierItems.Add(Entry);
			TotalWeight += Entry->Weight;
		}
	}

	// Fallback to common if tier is empty
	if (MatchingTierItems.Num() == 0)
	{
		// Logic to retry or fallback...
		return nullptr; 
	}

	// Weighted random selection
	float Roll = FMath::FRandRange(0.0f, TotalWeight);
	float CurrentWeightSum = 0.0f;
	FLootTableEntry* SelectedEntry = nullptr;

	for (auto* Entry : MatchingTierItems)
	{
		CurrentWeightSum += Entry->Weight;
		if (Roll <= CurrentWeightSum)
		{
			SelectedEntry = Entry;
			break;
		}
	}

	if (SelectedEntry && SelectedEntry->ItemClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AItemBase* SpawnedItem = GetWorld()->SpawnActor<AItemBase>(SelectedEntry->ItemClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (SpawnedItem)
		{
			SpawnedItem->ApplyLootTableEntry(*SelectedEntry);
		}
		return SpawnedItem;
	}

	return nullptr;
}

EItemRarity ALootSpawner::RollRarity(int32 Round, float Luck)
{
	FRarityWeights Weights = GenerationSettings->GetWeightsForRound(Round);

	// Apply Luck: Luck increases weights of high tiers
	// Simple multiplier: each point of luck adds 5% more weight to Rare/Legendary
	float LuckBonus = (Luck - 1.0f) * 0.05f; 
	
	float AdjustedRare = Weights.RareWeight * (1.0f + LuckBonus);
	float AdjustedLegendary = Weights.LegendaryWeight * (1.0f + (LuckBonus * 2.0f));

	float Total = Weights.CommonWeight + Weights.UncommonWeight + AdjustedRare + AdjustedLegendary;
	float Roll = FMath::FRandRange(0.0f, Total);

	if (Roll <= AdjustedLegendary) return EItemRarity::Legendary;
	if (Roll <= AdjustedLegendary + AdjustedRare) return EItemRarity::Rare;
	if (Roll <= AdjustedLegendary + AdjustedRare + Weights.UncommonWeight) return EItemRarity::Uncommon;
	
	return EItemRarity::Common;
}