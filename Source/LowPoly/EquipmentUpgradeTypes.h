#pragma once

#include "CoreMinimal.h"
#include "EquipmentUpgradeTypes.generated.h"

UENUM(BlueprintType)
enum class EItemUpgradeTarget : uint8
{
	Next,
	ToLegendary
};

USTRUCT(BlueprintType)
struct FUpgradePreviewStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString BeforeText;

	UPROPERTY(BlueprintReadOnly)
	FString AfterText;

	UPROPERTY(BlueprintReadOnly)
	float BeforePhysicalNegation = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float AfterPhysicalNegation = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float BeforeMagicNegation = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float AfterMagicNegation = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bIsNegationPreview = false;
};

