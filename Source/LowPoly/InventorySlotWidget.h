#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "ShopContextMenuWidget.h"
#include "InventorySlotWidget.generated.h"

class AItemBase;

UENUM(BlueprintType)
enum class EInventorySlotType : uint8
{
	MainHand,
	OffHand,
	Armor,
	Consumable,
	Bag,
    Ability,
    Perk
};

UCLASS()
class LOWPOLY_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget)) UImage* ItemIcon;
	UPROPERTY(meta = (BindWidget)) UTextBlock* ItemName;
	UPROPERTY(meta = (BindWidget)) UBorder* HighlightBorder;

	UPROPERTY(EditDefaultsOnly, Category = "CK|UI")
	TSubclassOf<class UShopContextMenuWidget> ContextMenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|UI")
	EContextType InteractionContext = EContextType::Drop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|UI")
	EInventorySlotType SlotType = EInventorySlotType::Bag;

    /** The index in the InventoryComponent array this slot represents (0, 1, 2, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|UI")
    int32 SlotIndex = 0;

	void UpdateSlot(AItemBase* Item, int32 NewSlotIndex);
	void SetHighlighted(bool bIsHighlighted);
	void SetActiveSlotIndicator(bool bIsActive);

private:
	/** True while mouse is hovering this slot widget (independent of whether it contains an item). */
	bool bIsHovered = false;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	AItemBase* CachedItem;
	float ClickStartTime;

private:
	bool IsItemCompatible(AItemBase* Item) const;
};
