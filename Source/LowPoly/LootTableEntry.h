#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemBase.h"
#include "LootTableEntry.generated.h"

USTRUCT(BlueprintType)
struct FLootTableEntry : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AItemBase> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemRarity Rarity;

	// Relative weight within its rarity tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BuyPrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SellPrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* ItemIcon;

	FLootTableEntry()
		: ItemClass(nullptr)
		, Rarity(EItemRarity::Common)
		, Weight(1.0f)
		, BuyPrice(50)
		, SellPrice(25)
		, Description(FText::GetEmpty())
		, ItemIcon(nullptr)
	{}
};
