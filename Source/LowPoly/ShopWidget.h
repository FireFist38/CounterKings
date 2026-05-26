#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopWidget.generated.h"

class UButton;
class UTextBlock;
class UShopSlotWidget;

UCLASS()
class LOWPOLY_API UShopWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget)) UShopSlotWidget* ShopSlot_0;
    UPROPERTY(meta = (BindWidget)) UShopSlotWidget* ShopSlot_1;
    UPROPERTY(meta = (BindWidget)) UShopSlotWidget* ShopSlot_2;
    UPROPERTY(meta = (BindWidget)) UShopSlotWidget* ShopSlot_3;
    UPROPERTY(meta = (BindWidget)) UShopSlotWidget* ShopSlot_4;

	UPROPERTY(meta = (BindWidget))
	UButton* RerollButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GoldText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimerText;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION() void OnSlotClicked(int32 SlotIndex);
	UFUNCTION() void OnRerollClicked();

public:
	void RefreshShopDisplay();
};
