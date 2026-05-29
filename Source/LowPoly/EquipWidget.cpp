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
		SlotWidget->OnSlotSelected.AddUniqueDynamic(this, &UEquipWidget::HandleSlotSelected);
	};

	// 1. Configure standard slots
	TArray<UInventorySlotWidget*> StandardSlots;
	StandardSlots.Add(MainHand_Active); StandardSlots.Add(OffHand_Active);
	StandardSlots.Add(MainHand_0); StandardSlots.Add(MainHand_1);
	StandardSlots.Add(OffHand_0); StandardSlots.Add(OffHand_1);
	StandardSlots.Add(ArmorSlot); StandardSlots.Add(ConsumableSlot);
	StandardSlots.Add(Ability_0); StandardSlots.Add(Ability_1); StandardSlots.Add(Ability_2);
	
	UInventorySlotWidget* PerkWidgets[] = { Perk_0, Perk_1, Perk_2, Perk_3, Perk_4, Perk_5, Perk_6, Perk_7, Perk_8, Perk_9 };
	for (UInventorySlotWidget* W : PerkWidgets) StandardSlots.Add(W);

	UInventorySlotWidget* BagWidgets[] = { Inv_0, Inv_1, Inv_2, Inv_3, Inv_4, Inv_5, Inv_6, Inv_7, Inv_8, Inv_9, Inv_10, Inv_11 };
	for (UInventorySlotWidget* W : BagWidgets) StandardSlots.Add(W);

	for (UInventorySlotWidget* CurrentSlot : StandardSlots)
	{
		if (!CurrentSlot) continue;
		
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
	}

	// 2. Configure Upgrade Tab slots
	TArray<UInventorySlotWidget*> UpgradeSlots;
	UpgradeSlots.Add(Upgrade_MainHand_Active); UpgradeSlots.Add(Upgrade_OffHand_Active);
	UpgradeSlots.Add(Upgrade_MainHand_0); UpgradeSlots.Add(Upgrade_MainHand_1);
	UpgradeSlots.Add(Upgrade_OffHand_0); UpgradeSlots.Add(Upgrade_OffHand_1);
	UpgradeSlots.Add(Upgrade_ArmorSlot); UpgradeSlots.Add(Upgrade_ConsumableSlot);
	
	UInventorySlotWidget* UpgradeBagWidgets[] = { 
		Upgrade_Inv_0, Upgrade_Inv_1, Upgrade_Inv_2, Upgrade_Inv_3, Upgrade_Inv_4, Upgrade_Inv_5, 
		Upgrade_Inv_6, Upgrade_Inv_7, Upgrade_Inv_8, Upgrade_Inv_9, Upgrade_Inv_10, Upgrade_Inv_11 
	};
	for (UInventorySlotWidget* W : UpgradeBagWidgets) UpgradeSlots.Add(W);

	for (UInventorySlotWidget* CurrentSlot : UpgradeSlots)
	{
		if (!CurrentSlot) continue;

		if (CurrentSlot == Upgrade_ArmorSlot) ConfigureSlot(CurrentSlot, EInventorySlotType::Armor);
		else if (CurrentSlot == Upgrade_ConsumableSlot) ConfigureSlot(CurrentSlot, EInventorySlotType::Consumable);
		else if (CurrentSlot == Upgrade_MainHand_Active || CurrentSlot == Upgrade_MainHand_0 || CurrentSlot == Upgrade_MainHand_1) ConfigureSlot(CurrentSlot, EInventorySlotType::MainHand);
		else if (CurrentSlot == Upgrade_OffHand_Active || CurrentSlot == Upgrade_OffHand_0 || CurrentSlot == Upgrade_OffHand_1) ConfigureSlot(CurrentSlot, EInventorySlotType::OffHand);
		else ConfigureSlot(CurrentSlot, EInventorySlotType::Bag);
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
	if (EquipTabSwitcher)
	{
		EquipTabSwitcher->SetActiveWidgetIndex(0);
		SetSlotsInteractionContext(EContextType::Sell);
	}
}

void UEquipWidget::OnUpgradeTabClicked()
{
	if (EquipTabSwitcher)
	{
		EquipTabSwitcher->SetActiveWidgetIndex(1);
		SetSlotsInteractionContext(EContextType::Drop); // Use Drop/Select context in upgrade tab
		
		// When switching to upgrade tab, clear selection and reset slot context
		SelectedUpgradeItem = nullptr;
		if (Upgrade_SourceSlot) Upgrade_SourceSlot->UpdateSlot(nullptr, 0);
		UpdateUpgradePreview();
	}
}

void UEquipWidget::SetSlotsInteractionContext(EContextType NewContext)
{
	// Collect every possible slot reference into a TArray to avoid compiler confusion with stack arrays of member variables
	TArray<UInventorySlotWidget*> AllPossibleSlots;
	
	AllPossibleSlots.Add(MainHand_Active); AllPossibleSlots.Add(OffHand_Active);
	AllPossibleSlots.Add(MainHand_0); AllPossibleSlots.Add(MainHand_1);
	AllPossibleSlots.Add(OffHand_0); AllPossibleSlots.Add(OffHand_1);
	AllPossibleSlots.Add(ArmorSlot); AllPossibleSlots.Add(ConsumableSlot);
	AllPossibleSlots.Add(Ability_0); AllPossibleSlots.Add(Ability_1); AllPossibleSlots.Add(Ability_2);
	
	UInventorySlotWidget* PWidgets[] = { Perk_0, Perk_1, Perk_2, Perk_3, Perk_4, Perk_5, Perk_6, Perk_7, Perk_8, Perk_9 };
	for (auto* W : PWidgets) AllPossibleSlots.Add(W);

	UInventorySlotWidget* BWidgets[] = { Inv_0, Inv_1, Inv_2, Inv_3, Inv_4, Inv_5, Inv_6, Inv_7, Inv_8, Inv_9, Inv_10, Inv_11 };
	for (auto* W : BWidgets) AllPossibleSlots.Add(W);

	AllPossibleSlots.Add(Upgrade_MainHand_Active); AllPossibleSlots.Add(Upgrade_OffHand_Active);
	AllPossibleSlots.Add(Upgrade_MainHand_0); AllPossibleSlots.Add(Upgrade_MainHand_1);
	AllPossibleSlots.Add(Upgrade_OffHand_0); AllPossibleSlots.Add(Upgrade_OffHand_1);
	AllPossibleSlots.Add(Upgrade_ArmorSlot); AllPossibleSlots.Add(Upgrade_ConsumableSlot);
	
	UInventorySlotWidget* UBWidgets[] = { 
		Upgrade_Inv_0, Upgrade_Inv_1, Upgrade_Inv_2, Upgrade_Inv_3, Upgrade_Inv_4, Upgrade_Inv_5, 
		Upgrade_Inv_6, Upgrade_Inv_7, Upgrade_Inv_8, Upgrade_Inv_9, Upgrade_Inv_10, Upgrade_Inv_11 
	};
	for (auto* W : UBWidgets) AllPossibleSlots.Add(W);

	for (UInventorySlotWidget* S : AllPossibleSlots)
	{
		if (S) S->InteractionContext = NewContext;
	}
}

void UEquipWidget::OnConfirmUpgradeClicked()
{
	if (!SelectedUpgradeItem) return;

	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (Character && Character->GetInventoryComponent())
	{
		Character->GetInventoryComponent()->Server_RequestUpgradeRarity(SelectedUpgradeItem, EItemUpgradeTarget::Next);
	}
}

void UEquipWidget::HandleSlotSelected(UInventorySlotWidget* SlotWidget)
{
	if (!EquipTabSwitcher || EquipTabSwitcher->GetActiveWidgetIndex() != 1) return;
	if (!SlotWidget) return;

	AItemBase* Item = SlotWidget->GetCachedItem();
	
	if (Item)
	{
		if (Item->IsA(APerkBase::StaticClass()) || Item->IsA(AAbilityBase::StaticClass()))
		{
			return;
		}
	}

	SelectedUpgradeItem = Item;

	if (Upgrade_SourceSlot)
	{
		Upgrade_SourceSlot->UpdateSlot(SelectedUpgradeItem, 0);
	}

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

	FUpgradePreviewStats Preview = UpgradeComp->BuildUpgradePreviewStats(Attr, SelectedUpgradeItem, CurrentRarity, TargetRarity);

	if (Upgrade_BeforeStats) Upgrade_BeforeStats->SetText(FText::FromString(Preview.BeforeText));
	
	if (Upgrade_AfterStats)
	{
		if (bHasCost)
		{
			Upgrade_AfterStats->SetText(FText::FromString(Preview.AfterText));
		}
		else
		{
			Upgrade_AfterStats->SetText(FText::GetEmpty());
		}
	}

	if (Btn_ConfirmUpgrade)
	{
		bool bCanAfford = Attr->GetGold() >= Cost;
		bool bIsEligible = bHasCost && CurrentRarity != EItemRarity::Legendary;
		Btn_ConfirmUpgrade->SetIsEnabled(bCanAfford && bIsEligible);
	}
}

void UEquipWidget::UpdateInventorySlots()
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (!Character || !Character->GetInventoryComponent()) return;

	UInventoryComponent* Inv = Character->GetInventoryComponent();
	
	int32 ActiveMH = Inv->GetActiveMainHandIndex();
	int32 ActiveOH = Inv->GetActiveOffHandIndex();
	AItemBase* MHItem = Inv->GetActiveMainHandItem();
	AItemBase* OHItem = Inv->GetActiveOffHandItem();

	const TArray<AItemBase*> MainHandSlots = Inv->GetMainHandSlots();
	const TArray<AItemBase*> OffHandSlots = Inv->GetOffHandSlots();
	AItemBase* Armor = Inv->GetArmorSet();
	AItemBase* Consumable = Inv->GetConsumableSlot();

	// Helper to update a slot and its optional duplicate
	auto UpdateDualSlot = [&](UInventorySlotWidget* Primary, UInventorySlotWidget* Secondary, AItemBase* Item, int32 Index, bool bIsActive = false)
	{
		if (Primary)
		{
			Primary->UpdateSlot(Item, Index);
			Primary->SetActiveSlotIndicator(bIsActive);
		}
		if (Secondary)
		{
			Secondary->UpdateSlot(Item, Index);
			Secondary->SetActiveSlotIndicator(bIsActive);
		}
	};

	UpdateDualSlot(MainHand_Active, Upgrade_MainHand_Active, MHItem, ActiveMH);
	UpdateDualSlot(OffHand_Active,  Upgrade_OffHand_Active, OHItem, ActiveOH);

	UpdateDualSlot(MainHand_0, Upgrade_MainHand_0, MainHandSlots.IsValidIndex(0) ? MainHandSlots[0] : nullptr, 0, ActiveMH == 0 && MHItem);
	UpdateDualSlot(MainHand_1, Upgrade_MainHand_1, MainHandSlots.IsValidIndex(1) ? MainHandSlots[1] : nullptr, 1, ActiveMH == 1 && MHItem);
	UpdateDualSlot(OffHand_0, Upgrade_OffHand_0, OffHandSlots.IsValidIndex(0) ? OffHandSlots[0] : nullptr, 0, ActiveOH == 0 && OHItem);
	UpdateDualSlot(OffHand_1, Upgrade_OffHand_1, OffHandSlots.IsValidIndex(1) ? OffHandSlots[1] : nullptr, 1, ActiveOH == 1 && OHItem);

	UpdateDualSlot(ArmorSlot, Upgrade_ArmorSlot, Armor, 0);
	UpdateDualSlot(ConsumableSlot, Upgrade_ConsumableSlot, Consumable, 0);

	// Bag Slots
	UInventorySlotWidget* PrimaryInv[] = { Inv_0, Inv_1, Inv_2, Inv_3, Inv_4, Inv_5, Inv_6, Inv_7, Inv_8, Inv_9, Inv_10, Inv_11 };
	UInventorySlotWidget* SecondaryInv[] = { Upgrade_Inv_0, Upgrade_Inv_1, Upgrade_Inv_2, Upgrade_Inv_3, Upgrade_Inv_4, Upgrade_Inv_5, Upgrade_Inv_6, Upgrade_Inv_7, Upgrade_Inv_8, Upgrade_Inv_9, Upgrade_Inv_10, Upgrade_Inv_11 };
	
	for (int32 i = 0; i < 12; i++)
	{
		UpdateDualSlot(PrimaryInv[i], SecondaryInv[i], Inv->GetBagItem(i), i);
	}

	// Ability Slots (Primary only for now)
	TArray<AAbilityBase*> Abilities = Inv->GetAbilitySlots();
	if (Ability_0) Ability_0->UpdateSlot(Abilities.IsValidIndex(0) ? (AItemBase*)Abilities[0] : nullptr, 0);
	if (Ability_1) Ability_1->UpdateSlot(Abilities.IsValidIndex(1) ? (AItemBase*)Abilities[1] : nullptr, 1);
	if (Ability_2) Ability_2->UpdateSlot(Abilities.IsValidIndex(2) ? (AItemBase*)Abilities[2] : nullptr, 2);

	// Perk Slots (Primary only for now)
	TArray<APerkBase*> Perks = Inv->GetPerkSlots();
	UInventorySlotWidget* PerkSlots[] = { Perk_0, Perk_1, Perk_2, Perk_3, Perk_4, Perk_5, Perk_6, Perk_7, Perk_8, Perk_9 };
	for (int32 i = 0; i < 10; i++)
	{
		if (PerkSlots[i]) PerkSlots[i]->UpdateSlot(Perks.IsValidIndex(i) ? (AItemBase*)Perks[i] : nullptr, i);
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
		if (SelectedUpgradeItem && SelectedUpgradeItem->GetRarity() != LastKnownRarity)
		{
			LastKnownRarity = SelectedUpgradeItem->GetRarity();
			UpdateUpgradePreview();
		}

		UpdateInventorySlots();
	}
}
