#include "CKRoundResultWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UCKRoundResultWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Start hidden or with fade in
	PlayFadeIn();
}

void UCKRoundResultWidget::SetupResult(bool bWin, int32 RoundNumber)
{
	if (ResultText)
	{
		ResultText->SetText(bWin ? FText::FromString("VICTORY") : FText::FromString("DEFEAT"));
		// Optional: Change color in C++ based on result
		ResultText->SetColorAndOpacity(bWin ? FLinearColor::Green : FLinearColor::Red);
	}

	if (RoundNumberText)
	{
		RoundNumberText->SetText(FText::Format(NSLOCTEXT("UI", "RoundFmt", "ROUND {0} COMPLETE"), FText::AsNumber(RoundNumber)));
	}
}

void UCKRoundResultWidget::PlayFadeIn()
{
	if (FadeAnim)
	{
		PlayAnimation(FadeAnim, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
	}
}

void UCKRoundResultWidget::PlayFadeOut()
{
	if (FadeAnim)
	{
		PlayAnimation(FadeAnim, 0.0f, 1, EUMGSequencePlayMode::Reverse, 1.0f);
	}
}
