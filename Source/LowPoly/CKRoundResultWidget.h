#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKRoundResultWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS(Abstract)
class LOWPOLY_API UCKRoundResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResultText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoundNumberText;

	// --- C++ Animations ---
	// Design requirement: Name these "FadeAnim" in your Blueprint widget animations list.
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeAnim;

	void SetupResult(bool bWin, int32 RoundNumber);

	// Plays Fade-In logic
	void PlayFadeIn();

	// Plays Fade-Out logic
	void PlayFadeOut();

protected:
	virtual void NativeConstruct() override;
};
