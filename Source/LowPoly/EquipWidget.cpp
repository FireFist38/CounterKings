#include "EquipWidget.h"
#include "InventorySlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "InventoryComponent.h"
#include "AbilityBase.h"
#include "PerkBase.h"
#include "ItemBase.h"
#include "CKGameMode.h"
#include "CKGameState.h"
#include "InventoryUpgradeComponent.h"
#include "Kismet/GameplayStatics.h"

void UEquipWidget::NativeConstruct()
{
	Super::NativeConstruct();

	auto ConfigureSlot = [this](UInventorySlotWidget* SlotWidget, EInventorySlotType NewSlotType)
	{
		if (!SlotWidget) return;
		SlotWidget->SlotType = NewSlotType;
		SlotWidget->InteractionContext = EContextType::Sell;

		// Bind selection delegate for all slots to handle upgrade selection
		// Note: We use AddUniqueDynamic to avoid double-binding if NativeConstruct is called multiple times
		// (though unlikely for UserWidgets, it's safer).
		// We'll use a member function for the binding.
	};

	TArray<UInventorySlotWidget*> AllSlots;
	AllSlots.Add(MainHand_Active); AllSlots.Add(OffHand_Active);
	AllSlots.Add(MainHand_0); AllSlots.Add(MainHand_1);
	AllSlots.Add(OffHand_0); AllSlots.Add(OffHand_1);
	AllSlots.Add(ArmorSlot); AllSlots.Add(ConsumableSlot);
	AllSlots.Add(Ability_0); AllSlots.Add(Ability_1); AllSlots.Add(Ability_2);
	
	UInventorySlotWidget* PerkWidgets[] = { Perk_0, Perk_1, Perk_2, Perk_3, Perk_4, Perk_5, Perk_6, Perk_7, Perk_8, Perk_9 };
	for (UInventorySlotWidget* W : PerkWidgets) AllSlots.Add(W);

	UInventorySlotWidget* BagWidgets[] = { Inv_0, Inv_1, Inv_2, Inv_3, Inv_4, Inv_5, Inv_6, Inv_7, Inv_8, Inv_9, Inv_10, Inv_11 };
	for (UInventorySlotWidget* W : BagWidgets) AllSlots.Add(W);

	for (UInventorySlotWidget* CurrentSlot : AllSlots)
	{
		if (!CurrentSlot) continue;
		
		// Configure basic slot types
		if (CurrentSlot == ArmorSlot) ConfigureSlot(CurrentSlot, EInventorySlotType::Armor);
		else if (CurrentSlot == ConsumableSlot) ConfigureSlot(CurrentSlot, EInventorySlotType::Consumable);
		else if (CurrentSlot == Ability_0 || CurrentSlot == Ability_1 || CurrentSlot == Ability_2) ConfigureSlot(CurrentSlot, EInventorySlotType::Ability);
		else if (CurrentSlot == MainHand_Active || CurrentSlot == MainHand_0 || CurrentSlot == MainHand_1) ConfigureSlot(CurrentSlot, EInventorySlotType::MainHand);
		else if (CurrentSlot == OffHand_Active || CurrentSlot == OffHand_0 || CurrentSlot == OffHand_1) ConfigureSlot(CurrentSlot, EInventorySlotType::OffHand);
		else
		{
			bool bIsPerk = false;
			for (UInventorySlotWidget* P : PerkWidgets) if (CurrentSlot == P) { bIsPerk = true; break; }
			ConfigureSlot(CurrentSlot, bIsPerk ? EInventorySlotType::Perk : EInventorySlotType::Bag);
		}

		// Bind selection
		CurrentSlot->OnSlotSelected.AddUniqueDynamic(this, &UEquipWidget::HandleSlotSelected);
	}

	// Bind Tab Buttons
	if (Btn_InventoryTab) Btn_InventoryTab->OnClicked.AddUniqueDynamic(this, &UEquipWidget::OnInventoryTabClicked);
	if (Btn_UpgradeTab) Btn_UpgradeTab->OnClicked.AddUniqueDynamic(this, &UEquipWidget::OnUpgradeTabClicked);
	if (Btn_ConfirmUpgrade) Btn_ConfirmUpgrade->OnClicked.AddUniqueDynamic(this, &UEquipWidget::OnConfirmUpgradeClicked);

	// Default to Inventory Tab
	OnInventoryTabClicked();
}

void UEquipWidget::OnInventoryTabClicked()
{
	if (EquipTabSwitcher) EquipTabSwitcher->SetActiveWidgetIndex(0);
}

void UEquipWidget::OnUpgradeTabClicked()
{
	if (EquipTabSwitcher) EquipTabSwitcher->SetActiveWidgetIndex(1);
}

void UEquipWidget::OnConfirmUpgradeClicked()
{
	if (!SelectedUpgradeItem) return;

	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (Character && Character->GetInventoryComponent())
	{
		// Send RPC to server
		Character->GetInventoryComponent()->Server_RequestUpgradeRarity(SelectedUpgradeItem, EItemUpgradeTarget::Next);
		
		// Note: The UI will refresh automatically via NativeTick/UpdateUpgradePreview 
		// as the item rarity replicates back from the server.
	}
}

void UEquipWidget::HandleSlotSelected(UInventorySlotWidget* SlotWidget)
{
	// Only care about selection if we are in the Upgrade tab
	if (!EquipTabSwitcher || EquipTabSwitcher->GetActiveWidgetIndex() != 1) return;

	if (!SlotWidget) return;

	AItemBase* Item = SlotWidget->GetCachedItem();
	
	// Eligibility check: Only non-perk/non-ability items can be upgraded (as per TODO.md)
	if (Item)
	{
		if (Item->IsA(APerkBase::StaticClass()) || Item->IsA(AAbilityBase::StaticClass()))
		{
			// Invalid selection for upgrade
			return;
		}
	}

	SelectedUpgradeItem = Item;

	// Update the source slot in the upgrade panel
	if (Upgrade_SourceSlot)
	{
		Upgrade_SourceSlot->UpdateSlot(SelectedUpgradeItem, 0);
	}

		// Trigger preview refresh
		if (SelectedUpgradeItem)
		{
			LastKnownRarity = SelectedUpgradeItem->GetRarity();
		}
		UpdateUpgradePreview();
}

void UEquipWidget::UpdateUpgradePreview()
{
	if (!UpgradePanel) return;

	if (!SelectedUpgradeItem)
	{
		if (Upgrade_BeforeStats) Upgrade_BeforeStats->SetText(FText::GetEmpty());
		if (Upgrade_AfterStats) Upgrade_AfterStats->SetText(FText::GetEmpty());
		if (Upgrade_CostText) Upgrade_CostText->SetText(FText::GetEmpty());
		if (Btn_ConfirmUpgrade) Btn_ConfirmUpgrade->SetIsEnabled(false);
		return;
	}

	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (!Character) return;

	UInventoryUpgradeComponent* UpgradeComp = Character->FindComponentByClass<UInventoryUpgradeComponent>();
	if (!UpgradeComp) return;

	UAttributeComponent* Attr = Character->GetAttributeComponent();
	if (!Attr) return;

	EItemRarity CurrentRarity = SelectedUpgradeItem->GetRarity();
	EItemRarity TargetRarity = UpgradeComp->ResolveTargetRarity(CurrentRarity, EItemUpgradeTarget::Next);

	// 1. Cost
	int32 Cost = 0;
	bool bHasCost = UpgradeComp->GetUpgradeCostFor(CurrentRarity, TargetRarity, Cost);
	
	if (Upgrade_CostText)
	{
		if (bHasCost)
		{
			Upgrade_CostText->SetText(FText::Format(FText::FromString(TEXT("Cost: {0} Gold")), FText::AsNumber(Cost)));
		}
		else
		{
			Upgrade_CostText->SetText(FText::FromString(TEXT("Max Rarity Reached")));
		}
	}

	// 2. Stats Preview
	// We use the BuildUpgradePreviewStats from the component
	FUpgradePreviewStats Preview = UpgradeComp->BuildUpgradePreviewStats(Attr, SelectedUpgradeItem, CurrentRarity, TargetRarity);

	if (Upgrade_BeforeStats) Upgrade_BeforeStats->SetText(FText::FromString(Preview.BeforeText));
	
	if (Upgrade_AfterStats)
	{
		if (bHasCost)
		{
			// Note: The component's BuildUpgradePreviewStats currently returns AfterText == BeforeText 
			// because it doesn't want to mutate the item. In a real scenario, we'd want to 
			// show the scaled-up values. For now, we follow the component's implementation.
			Upgrade_AfterStats->SetText(FText::FromString(Preview.AfterText));
		}
		else
		{
			Upgrade_AfterStats->SetText(FText::GetEmpty());
		}
	}

	// 3. Confirm Button State
	if (Btn_ConfirmUpgrade)
	{
		bool bCanAfford = Attr->GetGold() >= Cost;
		bool bIsEligible = bHasCost && CurrentRarity != EItemRarity::Legendary;
		Btn_ConfirmUpgrade->SetIsEnabled(bCanAfford && bIsEligible);
	}
}

void UEquipWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
	
	if (GS && TimerText) 
	{
		TimerText->SetText(FText::AsNumber(FMath::CeilToInt(GS->GetRemainingTime())));
	}
	
	if (Character && Character->GetAttributeComponent() && GoldText) 
	{
		GoldText->SetText(FText::AsNumber(Character->GetAttributeComponent()->GetGold()));
	}

	if (Character && Character->GetInventoryComponent())
	{
		UInventoryComponent* Inv = Character->GetInventoryComponent();
		
			// If the selected item's rarity has changed (replicated from server), refresh the preview
			if (SelectedUpgradeItem && SelectedUpgradeItem->GetRarity() != LastKnownRarity)
			{
				LastKnownRarity = SelectedUpgradeItem->GetRarity();
				UpdateUpgradePreview();
			}

		int32 ActiveMH = Inv->GetActiveMainHandIndex();
		int32 ActiveOH = Inv->GetActiveOffHandIndex();

		// Update Equipped Slots (Active Only)
		if (MainHand_Active) MainHand_Active->UpdateSlot(Inv->GetActiveMainHandItem(), ActiveMH);
		if (OffHand_Active)  OffHand_Active->UpdateSlot(Inv->GetActiveOffHandItem(), ActiveOH);
		const TArray<AItemBase*> MainHandSlots = Inv->GetMainHandSlots();
		const TArray<AItemBase*> OffHandSlots = Inv->GetOffHandSlots();

		if (MainHand_0)
		{
			AItemBase* MainHandItem0 = MainHandSlots.IsValidIndex(0) ? MainHandSlots[0] : nullptr;
			MainHand_0->UpdateSlot(MainHandItem0, 0);
			MainHand_0->SetActiveSlotIndicator(ActiveMH == 0 && MainHandItem0);
		}
		if (MainHand_1)
		{
			AItemBase* MainHandItem1 = MainHandSlots.IsValidIndex(1) ? MainHandSlots[1] : nullptr;
			MainHand_1->UpdateSlot(MainHandItem1, 1);
			MainHand_1->SetActiveSlotIndicator(ActiveMH == 1 && MainHandItem1);
		}
		if (OffHand_0)
		{
			AItemBase* OffHandItem0 = OffHandSlots.IsValidIndex(0) ? OffHandSlots[0] : nullptr;
			OffHand_0->UpdateSlot(OffHandItem0, 0);
			OffHand_0->SetActiveSlotIndicator(ActiveOH == 0 && OffHandItem0);
		}
		if (OffHand_1)
		{
			AItemBase* OffHandItem1 = OffHandSlots.IsValidIndex(1) ? OffHandSlots[1] : nullptr;
			OffHand_1->UpdateSlot(OffHandItem1, 1);
			OffHand_1->SetActiveSlotIndicator(ActiveOH == 1 && OffHandItem1);
		}
		if (ArmorSlot)       ArmorSlot->UpdateSlot(Inv->GetArmorSet(), 0);
		if (ConsumableSlot)  ConsumableSlot->UpdateSlot(Inv->GetConsumableSlot(), 0);

        // Update Ability Slots
        TArray<AAbilityBase*> Abilities = Inv->GetAbilitySlots();
        if (Ability_0) Ability_0->UpdateSlot(Abilities.IsValidIndex(0) ? (AItemBase*)Abilities[0] : nullptr, 0);
        if (Ability_1) Ability_1->UpdateSlot(Abilities.IsValidIndex(1) ? (AItemBase*)Abilities[1] : nullptr, 1);
        if (Ability_2) Ability_2->UpdateSlot(Abilities.IsValidIndex(2) ? (AItemBase*)Abilities[2] : nullptr, 2);

        // Update Perk Slots
        TArray<APerkBase*> Perks = Inv->GetPerkSlots();
        UInventorySlotWidget* PerkSlots[] = { Perk_0, Perk_1, Perk_2, Perk_3, Perk_4, Perk_5, Perk_6, Perk_7, Perk_8, Perk_9 };
        for (int32 i = 0; i < 10; i++)
        {
            if (PerkSlots[i]) PerkSlots[i]->UpdateSlot(Perks.IsValidIndex(i) ? (AItemBase*)Perks[i] : nullptr, i);
        }
		
		// Update Inventory Bag Slots (Inv_0 to Inv_11)
		UInventorySlotWidget* InvSlots[] = { Inv_0, Inv_1, Inv_2, Inv_3, Inv_4, Inv_5, Inv_6, Inv_7, Inv_8, Inv_9, Inv_10, Inv_11 };
		for (int32 i = 0; i < 12; i++)
		{
			if (InvSlots[i])
			{
				InvSlots[i]->UpdateSlot(Inv->GetBagItem(i), i);
			}
		}
	}
}
