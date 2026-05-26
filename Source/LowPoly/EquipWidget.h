#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "EquipWidget.generated.h"

class UButton;
class UInventorySlotWidget;

UCLASS()
class LOWPOLY_API UEquipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- Equipped Slots ---
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* MainHand_Active;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* OffHand_Active;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* MainHand_0;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* MainHand_1;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* OffHand_0;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* OffHand_1;

	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* ArmorSlot;
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* ConsumableSlot;

    // --- Ability Slots ---
    UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Ability_0;
    UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Ability_1;
    UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Ability_2;

    // --- Perk Slots (10 Total) ---
    UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Perk_0;
    UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Perk_1;
    UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Perk_2;
    UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Perk_3;
    UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Perk_4;
    UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Perk_5;
    UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Perk_6;
    UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Perk_7;
    UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Perk_8;
    UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Perk_9;

	// 12 Fixed Inventory Slots
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* Inv_0;
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* Inv_1;
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* Inv_2;
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* Inv_3;
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* Inv_4;
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* Inv_5;
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* Inv_6;
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* Inv_7;
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* Inv_8;
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* Inv_9;
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* Inv_10;
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* Inv_11;

	UPROPERTY(meta = (BindWidget)) UTextBlock* TimerText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* GoldText;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
