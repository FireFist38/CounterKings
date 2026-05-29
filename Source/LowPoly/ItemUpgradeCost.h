#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "Engine/DataTable.h"
#include "ItemUpgradeCost.generated.h"

USTRUCT(BlueprintType)
struct FItemUpgradeCostRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemRarity CurrentRarity = EItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemRarity NextRarity = EItemRarity::Uncommon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 GoldCost = 0;
};

