#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemTooltipWidget.generated.h"

class UTextBlock;
class AItemBase;

UCLASS()
class LOWPOLY_API UItemTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DescriptionText; // Placeholder if you add item descriptions

	void SetTooltipData(AItemBase* Item);
};
