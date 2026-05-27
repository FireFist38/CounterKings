#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "PreRoundRootWidget.generated.h"

class UButton;
class UWidgetSwitcher;

UCLASS()
class LOWPOLY_API UPreRoundRootWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- Main Navigation ---
	UPROPERTY(meta = (BindWidget))
	UButton* NavShopButton;
	UPROPERTY(meta = (BindWidget))
	UButton* NavLevelUpButton;
	UPROPERTY(meta = (BindWidget))
	UButton* NavEquipButton;
	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ReadyUpButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimerText;

	// --- Content Switching ---
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* ContentSwitcher;

    /** Programmatically switches to a specific tab */
    void SwitchToTab(int32 Index);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION() void ShowShop();
	UFUNCTION() void ShowLevelUp();
	UFUNCTION() void ShowEquip();
	UFUNCTION() void GoBackToMenu();
	UFUNCTION() void ReadyUp();
};
