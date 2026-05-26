#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemBase.h"
#include "LootGenerationData.generated.h"

USTRUCT(BlueprintType)
struct FRarityWeights
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CommonWeight = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UncommonWeight = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RareWeight = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LegendaryWeight = 1.0f;
};

UCLASS()
class LOWPOLY_API ULootGenerationData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Map of StartRound to Weights (e.g., Round 1-5, 6-10, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TMap<int32, FRarityWeights> RoundBasedWeights;

	FRarityWeights GetWeightsForRound(int32 Round) const
	{
		int32 BestMatch = 1;
		for (auto& Elem : RoundBasedWeights)
		{
			if (Round >= Elem.Key && Elem.Key >= BestMatch)
			{
				BestMatch = Elem.Key;
			}
		}
		return RoundBasedWeights.Contains(BestMatch) ? RoundBasedWeights[BestMatch] : FRarityWeights();
	}
};
