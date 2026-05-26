#include "InventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "ItemBase.h"
#include "AbilityBase.h"
#include "PerkBase.h"
#include "CKGameState.h"
#include "LootTableEntry.h"
#include "ShopContextMenuWidget.h"
#include "ItemDragDropOperation.h"
#include "PlayerCharacter.h"
#include "InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "MainHandBase.h"
#include "OffHandBase.h"
#include "ShieldBase.h"
#include "TomeBase.h"
#include "SecondaryRangedBase.h"
#include "ArmorBase.h"
#include "ConsumableBase.h"

void UInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	bIsHovered = false;
	SetHighlighted(false);
}

void UInventorySlotWidget::SetHighlighted(bool bIsHighlighted)
{
	if (HighlightBorder)
	{
		HighlightBorder->SetVisibility(bIsHighlighted ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UInventorySlotWidget::SetActiveSlotIndicator(bool bIsActive)
{
	// Active indicator should not override hover highlighting.
	// The highlight border is controlled by hover (NativeOnMouseEnter/Leave).
}

void UInventorySlotWidget::UpdateSlot(AItemBase* Item, int32 NewSlotIndex)
{
	CachedItem = Item;
	SlotIndex = NewSlotIndex;

	if (!Item)
	{
		SetVisibility(ESlateVisibility::Visible);
		if (ItemIcon) ItemIcon->SetOpacity(0.0f);
		if (ItemName) ItemName->SetText(FText::GetEmpty());
		SetHighlighted(false);
		return;
	}

	SetVisibility(ESlateVisibility::Visible);
	ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
	
	TArray<UDataTable*> SearchTables;
	if (GS)
	{
		SearchTables.Append(GS->GetAllItemTables());
	}

	TArray<FString> FallbackTablePaths = {
		TEXT("/Game/Data/DT_Perks"),
		TEXT("/Game/Data/DT_Weapons"),
		TEXT("/Game/Data/DT_Armor"),
		TEXT("/Game/Data/DT_Abilities"),
		TEXT("/Game/Data/DT_Loot")
	};

	for (const FString& Path : FallbackTablePaths)
	{
		UDataTable* LoadedTable = LoadObject<UDataTable>(nullptr, *Path);
		if (LoadedTable && !SearchTables.Contains(LoadedTable))
		{
			SearchTables.Add(LoadedTable);
		}
	}

	FString ItemClassName = Item->GetClass()->GetPathName();
	UClass* ItemClass = Item->GetClass();

	for (UDataTable* Table : SearchTables)
	{
		if (!Table) continue;

		TArray<FLootTableEntry*> Rows;
		Table->GetAllRows<FLootTableEntry>(TEXT("InventoryLookup"), Rows);
		for (auto* Row : Rows)
		{
			if (Row->ItemClass && (Row->ItemClass == ItemClass || Row->ItemClass->GetPathName() == ItemClassName))
			{
				if (ItemName) ItemName->SetText(Row->ItemName);
				if (ItemIcon && Row->ItemIcon)
				{
					ItemIcon->SetBrushFromTexture(Row->ItemIcon);
					ItemIcon->SetOpacity(1.0f);
				}
				return;
			}
		}
	}

	// Fallback if not found in any registered data table
	if (ItemName)
	{
		ItemName->SetText(FText::FromName(Item->GetItemName()));
	}
	if (ItemIcon)
	{
		ItemIcon->SetOpacity(0.0f);
	}
}

void UInventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	bIsHovered = true;
	SetHighlighted(true);
}

void UInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	bIsHovered = false;
	SetHighlighted(false);
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && CachedItem)
    {
        ClickStartTime = GetWorld()->GetTimeSeconds();
        return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }
    return FReply::Unhandled();
}

FReply UInventorySlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (GetWorld()->GetTimeSeconds() - ClickStartTime < 0.2f && CachedItem)
    {
        APlayerController* PC = GetOwningPlayer();
        if (PC && ContextMenuClass)
        {
            TArray<UUserWidget*> FoundWidgets;
            UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UShopContextMenuWidget::StaticClass(), false);
            for (UUserWidget* Widget : FoundWidgets) Widget->RemoveFromParent();

            UShopContextMenuWidget* Menu = CreateWidget<UShopContextMenuWidget>(PC, ContextMenuClass);
            if (Menu)
            {
                // Track the currently open context menu so it can be closed when parent UI closes
                if (APlayerCharacter* Character = Cast<APlayerCharacter>(PC->GetPawn()))
                {
                    // If we already had a tracked menu, clear it before replacing
                    Character->ActiveContextMenuInstance = Menu;
                }

				ESlotGroup SourceGroup = ESlotGroup::Bag;
				switch (SlotType)
				{
					case EInventorySlotType::MainHand: SourceGroup = ESlotGroup::MainHand; break;
					case EInventorySlotType::OffHand: SourceGroup = ESlotGroup::OffHand; break;
					case EInventorySlotType::Armor: SourceGroup = ESlotGroup::Armor; break;
					case EInventorySlotType::Consumable: SourceGroup = ESlotGroup::Consumable; break;
                    case EInventorySlotType::Ability: SourceGroup = ESlotGroup::Ability; break;
                    case EInventorySlotType::Perk: SourceGroup = ESlotGroup::Perk; break;
					default: SourceGroup = ESlotGroup::Bag; break;
				}

				EContextType ActiveContext = InteractionContext;
				ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
				if (GS && GS->GetMatchPhase() == ECKMatchPhase::PostRound && SlotType == EInventorySlotType::Bag)
				{
					ActiveContext = EContextType::Sell;
				}

				Menu->SetupContextMenu(CachedItem, SlotIndex, ActiveContext, SourceGroup);
				Menu->AddToViewport(1000);

				// Force a layout prepass so GetDesiredSize() returns the correct size.
				Menu->ForceLayoutPrepass();

				// Use DPI-scaled mouse position so clamping matches what SetPositionInViewport expects.
				FVector2D MousePos;
				PC->GetMousePosition(MousePos.X, MousePos.Y);

				const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
				const FVector2D MousePosScaled = MousePos / FMath::Max(0.01f, ViewportScale);

				// Clamp context menu so it never spawns off-screen near viewport edges.
				const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
				const FVector2D DesiredSize = Menu->GetDesiredSize();

				const float MaxX = FMath::Max(0.0f, (float)ViewportSize.X - (float)DesiredSize.X);
				const float MaxY = FMath::Max(0.0f, (float)ViewportSize.Y - (float)DesiredSize.Y);

				const float ClampedX = FMath::Clamp((float)MousePosScaled.X, 0.0f, MaxX);
				const float ClampedY = FMath::Clamp((float)MousePosScaled.Y, 0.0f, MaxY);

				Menu->SetPositionInViewport(FVector2D(ClampedX, ClampedY), false);
            }
        }
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    // If a context menu is open, close it as soon as the user starts dragging.
    // Otherwise the menu can remain on-screen at its old position while UI changes.
    if (APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
    {
        PC->CloseContextMenu();
    }

    UItemDragDropOperation* DragOp = NewObject<UItemDragDropOperation>();
    DragOp->ItemBeingDragged = CachedItem;
    DragOp->SourceSlotIndex = SlotIndex;
    
    switch (SlotType)
    {
        case EInventorySlotType::MainHand: DragOp->SourceSlotGroup = ESlotGroup::MainHand; break;
        case EInventorySlotType::OffHand: DragOp->SourceSlotGroup = ESlotGroup::OffHand; break;
        case EInventorySlotType::Armor: DragOp->SourceSlotGroup = ESlotGroup::Armor; break;
        case EInventorySlotType::Consumable: DragOp->SourceSlotGroup = ESlotGroup::Consumable; break;
        case EInventorySlotType::Ability: DragOp->SourceSlotGroup = ESlotGroup::Ability; break;
        case EInventorySlotType::Perk: DragOp->SourceSlotGroup = ESlotGroup::Perk; break;
        default: DragOp->SourceSlotGroup = ESlotGroup::Bag; break;
    }

    UUserWidget* VisualWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), GetClass());
    if (VisualWidget)
    {
        DragOp->DefaultDragVisual = VisualWidget;
    }

    OutOperation = DragOp;
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UItemDragDropOperation* DragOp = Cast<UItemDragDropOperation>(InOperation);
    if (!DragOp || !DragOp->ItemBeingDragged) return false;

    if (!IsItemCompatible(DragOp->ItemBeingDragged)) return false;

    APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (PC && PC->GetInventoryComponent())
    {
        ESlotGroup TargetGroup;
        switch (SlotType)
        {
            case EInventorySlotType::MainHand: TargetGroup = ESlotGroup::MainHand; break;
            case EInventorySlotType::OffHand: TargetGroup = ESlotGroup::OffHand; break;
            case EInventorySlotType::Armor: TargetGroup = ESlotGroup::Armor; break;
            case EInventorySlotType::Consumable: TargetGroup = ESlotGroup::Consumable; break;
            case EInventorySlotType::Ability: TargetGroup = ESlotGroup::Ability; break;
            case EInventorySlotType::Perk: TargetGroup = ESlotGroup::Perk; break;
            default: TargetGroup = ESlotGroup::Bag; break;
        }

        PC->GetInventoryComponent()->Server_MoveItem(DragOp->SourceSlotGroup, DragOp->SourceSlotIndex, TargetGroup, SlotIndex);
        return true;
    }

    return false;
}

bool UInventorySlotWidget::IsItemCompatible(AItemBase* Item) const
{
    if (!Item) return false;

    switch (SlotType)
    {
        case EInventorySlotType::MainHand:
            return Item->IsA(AMainHandBase::StaticClass());
        case EInventorySlotType::OffHand:
            return Item->IsA(AShieldBase::StaticClass()) || 
                   Item->IsA(ATomeBase::StaticClass()) || 
                   Item->IsA(ASecondaryRangedBase::StaticClass());
        case EInventorySlotType::Armor:
            return Item->IsA(AArmorBase::StaticClass());
        case EInventorySlotType::Consumable:
            return Item->IsA(AConsumableBase::StaticClass());
        case EInventorySlotType::Ability:
            return Item->IsA(AAbilityBase::StaticClass());
        case EInventorySlotType::Perk:
            return Item->IsA(APerkBase::StaticClass());
        case EInventorySlotType::Bag:
            return true;
        default:
            return false;
    }
}
