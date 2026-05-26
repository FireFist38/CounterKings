#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKScoreboardWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class LOWPOLY_API UCKScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- Header ---
	UPROPERTY(meta = (BindWidget)) UTextBlock* RoundText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* TimerText;

    // --- 10 Player Slots ---
    #define DECLARE_SLOT_WIDGETS(Index) \
    UPROPERTY(meta = (BindWidget)) UTextBlock* Name##Index; \
    UPROPERTY(meta = (BindWidget)) UProgressBar* HPBar##Index; \
    UPROPERTY(meta = (BindWidget)) UTextBlock* HPVal##Index; \
    UPROPERTY(meta = (BindWidget)) UTextBlock* Gold##Index;

    DECLARE_SLOT_WIDGETS(0)
    DECLARE_SLOT_WIDGETS(1)
    DECLARE_SLOT_WIDGETS(2)
    DECLARE_SLOT_WIDGETS(3)
    DECLARE_SLOT_WIDGETS(4)
    DECLARE_SLOT_WIDGETS(5)
    DECLARE_SLOT_WIDGETS(6)
    DECLARE_SLOT_WIDGETS(7)
    DECLARE_SLOT_WIDGETS(8)
    DECLARE_SLOT_WIDGETS(9)

    #undef DECLARE_SLOT_WIDGETS

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    void UpdateScoreboard();
};
