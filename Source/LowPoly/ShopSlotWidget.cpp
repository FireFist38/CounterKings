#include "ShopSlotWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/CheckBox.h"
#include "ItemBase.h"
#include "ShopContextMenuWidget.h"
#include "CKGameState.h"
#include "LootTableEntry.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UShopSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetHighlighted(false);
	
	if (ItemButton)
	{
		ItemButton->OnClicked.AddDynamic(this, &UShopSlotWidget::OnItemClicked);
	}

    if (LockBox)
    {
        LockBox->OnCheckStateChanged.AddDynamic(this, &UShopSlotWidget::OnLockToggled);
    }
}

FReply UShopSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (ActiveContextMenu)
    {
        ActiveContextMenu->RemoveFromParent();
        ActiveContextMenu = nullptr;
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UShopSlotWidget::OnItemClicked()
{
    if (ActiveContextMenu)
    {
        ActiveContextMenu->RemoveFromParent();
        ActiveContextMenu = nullptr;
    }

	if (ContextMenuClass && CachedItem)
	{
		APlayerController* PC = GetOwningPlayer();
		if (!PC) return;

		ActiveContextMenu = CreateWidget<UShopContextMenuWidget>(PC, ContextMenuClass);
		if (ActiveContextMenu)
		{
			ActiveContextMenu->SetupContextMenu(CachedItem, CachedSlotIndex, EContextType::Purchase, ESlotGroup::Bag);
			ActiveContextMenu->AddToViewport(1000);
            ActiveContextMenu->SetVisibility(ESlateVisibility::Visible);

			FVector2D MousePos;
			PC->GetMousePosition(MousePos.X, MousePos.Y);

            FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());
            FVector2D MenuSize = FVector2D(200.0f, 150.0f);

            float ClampedX = FMath::Clamp(MousePos.X, 0.0f, ViewportSize.X - MenuSize.X);
            float ClampedY = FMath::Clamp(MousePos.Y, 0.0f, ViewportSize.Y - MenuSize.Y);

			ActiveContextMenu->SetPositionInViewport(FVector2D(ClampedX, ClampedY));
            
            APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
            if (Character) Character->ActiveContextMenuInstance = ActiveContextMenu;
		}
	}
}

void UShopSlotWidget::OnLockToggled(bool bIsChecked)
{
    APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (Character)
    {
        Character->Server_ToggleLockSlot(CachedSlotIndex);
    }
}

void UShopSlotWidget::SetHighlighted(bool bIsHighlighted)
{
	if (HighlightBorder)
	{
		HighlightBorder->SetVisibility(bIsHighlighted ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UShopSlotWidget::SetupSlot(AItemBase* Item, int32 SlotIndex, int32 Price)
{
	CachedItem = Item;
	CachedSlotIndex = SlotIndex;
    
    // Set LockBox state
    APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (Character && LockBox)
    {
        if (Character->LockedSlots.IsValidIndex(SlotIndex))
        {
            LockBox->SetIsChecked(Character->LockedSlots[SlotIndex]);
        }
    }
    
	if (PriceText)
	{
		PriceText->SetText(Item ? FText::AsNumber(Price) : FText::GetEmpty());
	}

	if (ItemIcon)
	{
        if (Item)
        {
            ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
            bool bFoundIcon = false;
            
            if (GS && GS->GetLootDataTable())
            {
                TArray<FLootTableEntry*> Rows;
                GS->GetLootDataTable()->GetAllRows<FLootTableEntry>(TEXT("IconLookup"), Rows);
                for(auto* Row : Rows)
                {
                    if (Row->ItemClass == Item->GetClass())
                    {
                        if (Row->ItemIcon)
                        {
                            ItemIcon->SetBrushFromTexture(Row->ItemIcon);
                            ItemIcon->SetOpacity(1.0f);
                            bFoundIcon = true;
                            break;
                        }
                    }
                }
            }
            
            if (!bFoundIcon) ItemIcon->SetOpacity(0.0f);
        }
        else
        {
            ItemIcon->SetOpacity(0.0f); 
        }
	}
}
