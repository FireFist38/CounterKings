#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.h"
#include "LootTableEntry.h"
#include "LootSpawner.generated.h"

class ULootGenerationData;
class UDataTable;

UCLASS()
class LOWPOLY_API ALootSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ALootSpawner();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Loot")
	TArray<UDataTable*> LootTables;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Loot")
	ULootGenerationData* GenerationSettings;

	// Visual helper for editor
	UPROPERTY(VisibleAnywhere)
	class UBillboardComponent* EditorVisualizer;

    // --- DEBUG ---
    // If set, this item will always be spawned instead of rolling from the table.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Loot|Debug")
    TSubclassOf<AItemBase> DebugGuaranteedItem;

public:	
	UFUNCTION(BlueprintCallable, Category = "CK|Loot")
	AItemBase* SpawnLoot(int32 CurrentRound, float PlayerLuck = 1.0f);

private:
	EItemRarity RollRarity(int32 Round, float Luck);
};
