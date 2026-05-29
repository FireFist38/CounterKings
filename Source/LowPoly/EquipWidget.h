#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ItemBase.h"
#include "InventoryComponent.h"
#include "EquipWidget.generated.h"

class UButton;
class UInventorySlotWidget;
class UWidgetSwitcher;
class UWidget;
class AItemBase;

UCLASS()
class LOWPOLY_API UEquipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- Equipped Slots (Inventory Tab) ---
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

	// 12 Fixed Inventory Slots (Inventory Tab)
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

	// --- Upgrade Tab Slots (Optional, for duplicate layout in Upgrade Panel) ---
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_MainHand_Active;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_OffHand_Active;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_MainHand_0;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_MainHand_1;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_OffHand_0;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_OffHand_1;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_ArmorSlot;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_ConsumableSlot;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_Inv_0;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_Inv_1;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_Inv_2;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_Inv_3;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_Inv_4;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_Inv_5;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_Inv_6;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_Inv_7;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_Inv_8;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_Inv_9;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_Inv_10;
	UPROPERTY(meta = (BindWidgetOptional)) UInventorySlotWidget* Upgrade_Inv_11;

	UPROPERTY(meta = (BindWidget)) UTextBlock* TimerText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* GoldText;

	// --- Upgrade Tab Widgets ---
	UPROPERTY(meta = (BindWidget)) UWidgetSwitcher* EquipTabSwitcher;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_InventoryTab;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_UpgradeTab;

	UPROPERTY(meta = (BindWidget)) UWidget* UpgradePanel;
	UPROPERTY(meta = (BindWidget)) UInventorySlotWidget* Upgrade_SourceSlot;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Upgrade_BeforeStats;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Upgrade_AfterStats;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Upgrade_CostText;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_ConfirmUpgrade;

protected:
	UFUNCTION() void OnInventoryTabClicked();
	UFUNCTION() void OnUpgradeTabClicked();
	UFUNCTION() void OnConfirmUpgradeClicked();
	UFUNCTION() void HandleSlotSelected(UInventorySlotWidget* SlotWidget);
	
	void UpdateUpgradePreview();
	void UpdateInventorySlots();
	void SetSlotsInteractionContext(EContextType NewContext);

	// Selection state for upgrade
	UPROPERTY() AItemBase* SelectedUpgradeItem = nullptr;

	// Keep track of the current rarity of the selected item to detect changes
	EItemRarity LastKnownRarity;

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
