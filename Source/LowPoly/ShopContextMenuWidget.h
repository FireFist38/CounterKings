#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryComponent.h"
#include "ShopContextMenuWidget.generated.h"

class UButton;
class UTextBlock;
class AItemBase;

class UImage;

UCLASS()
class LOWPOLY_API UShopContextMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget)) UTextBlock* ItemNameText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* DescriptionText;
    
	UPROPERTY(meta = (BindWidget)) UTextBlock* DamageText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* BuyPriceText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* SellPriceText;

	UPROPERTY(meta = (BindWidget)) UButton* ActionButton;
	UPROPERTY(meta = (BindWidget)) UButton* CloseButton;
	UPROPERTY(meta = (BindWidget)) UTextBlock* ActionButtonText;

	// Ranged stats (shown for secondary ranged / guns)
	UPROPERTY(meta = (BindWidget)) UTextBlock* RateOfFireText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* MagazineCapacityText;

	// Staff spell icons (shown for magic weapons)
	UPROPERTY(meta = (BindWidget)) UImage* PrimarySpellIcon;
	UPROPERTY(meta = (BindWidget)) UImage* SecondarySpellIcon;

	void SetupContextMenu(AItemBase* Item, int32 SlotIndex, EContextType ContextType, ESlotGroup SourceGroup = ESlotGroup::Bag);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION() void OnActionClicked();
	UFUNCTION() void OnCloseClicked();

	UPROPERTY()
	AItemBase* CachedItem;
	
	int32 CachedSlotIndex;
	EContextType CurrentType;
	ESlotGroup CachedSourceGroup;
};
