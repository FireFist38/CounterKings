#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InventoryComponent.h" // For ESlotGroup
#include "ItemDragDropOperation.generated.h"

class AItemBase;

UCLASS()
class LOWPOLY_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	AItemBase* ItemBeingDragged;

	UPROPERTY(BlueprintReadWrite)
	int32 SourceSlotIndex;

	UPROPERTY(BlueprintReadWrite)
	ESlotGroup SourceSlotGroup;
};
