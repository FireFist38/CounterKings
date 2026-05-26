#include "CKXPBarWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/ProgressBar.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/WidgetTree.h"

void UCKXPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
    UE_LOG(LogTemp, Log, TEXT("CKXPBarWidget: NativeConstruct called. Container valid: %s"), SegmentContainer ? TEXT("True") : TEXT("False"));
}

void UCKXPBarWidget::UpdateXPBar(int32 Level, float CurrentXP, float XPPerLevel)
{
	if (!SegmentContainer) 
    {
        UE_LOG(LogTemp, Warning, TEXT("CKXPBarWidget: UpdateXPBar called but SegmentContainer is NULL!"));
        return;
    }

	// Update existing segments or rebuild
	if (SegmentContainer->GetChildrenCount() != Level)
	{
        UE_LOG(LogTemp, Log, TEXT("CKXPBarWidget: Rebuilding %d segments (current count: %d)"), Level, SegmentContainer->GetChildrenCount());
        SegmentContainer->ClearChildren();

        for (int32 i = 0; i < Level; ++i)
        {
            UProgressBar* Segment = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
            if (Segment)
            {
                // Force visibility and basic style
                Segment->SetVisibility(ESlateVisibility::Visible);
                Segment->SetFillColorAndOpacity(FLinearColor::Yellow); 
                Segment->SetPercent(0.0f);

                UHorizontalBoxSlot* NewSlot = SegmentContainer->AddChildToHorizontalBox(Segment);
                NewSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                NewSlot->SetPadding(FMargin(2.0f, 0.0f));
            }
        }
    }

    // Update segment percentages
    for (int32 i = 0; i < SegmentContainer->GetChildrenCount(); ++i)
    {
        UProgressBar* Segment = Cast<UProgressBar>(SegmentContainer->GetChildAt(i));
        if (Segment)
        {
            float SegmentXP = FMath::Clamp(CurrentXP - (i * XPPerLevel), 0.0f, XPPerLevel);
            Segment->SetPercent(SegmentXP / XPPerLevel);
        }
    }
}
