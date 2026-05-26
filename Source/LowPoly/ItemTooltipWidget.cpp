#include "ItemTooltipWidget.h"
#include "Components/TextBlock.h"
#include "ItemBase.h"

void UItemTooltipWidget::SetTooltipData(AItemBase* Item)
{
	if (Item)
	{
		if (NameText) NameText->SetText(FText::FromName(Item->GetItemName()));
        if (DescriptionText) DescriptionText->SetText(Item->GetDescription());
	}
}
