#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopSlotWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UBorder;
class UCheckBox;
class AItemBase;

UCLASS()
class LOWPOLY_API UShopSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UButton* ItemButton;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PriceText;

	UPROPERTY(meta = (BindWidget))
	UBorder* HighlightBorder;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* LockBox;

public:
	UPROPERTY(EditDefaultsOnly, Category = "CK|UI")
	TSubclassOf<class UShopContextMenuWidget> ContextMenuClass;

	void SetupSlot(AItemBase* Item, int32 SlotIndex, int32 Price);
	void SetHighlighted(bool bIsHighlighted);

protected:
	virtual void NativeConstruct() override;

    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION() void OnItemClicked();
    UFUNCTION() void OnLockToggled(bool bIsChecked);

	AItemBase* CachedItem;
	int32 CachedSlotIndex;

    UPROPERTY()
    class UShopContextMenuWidget* ActiveContextMenu;
};
