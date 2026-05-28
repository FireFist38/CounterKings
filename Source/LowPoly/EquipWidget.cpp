#include "EquipWidget.h"
#include "InventorySlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "InventoryComponent.h"
#include "AbilityBase.h"
#include "PerkBase.h"
#include "ItemBase.h"
#include "CKGameMode.h"
#include "CKGameState.h"
#include "Kismet/GameplayStatics.h"

void UEquipWidget::NativeConstruct()
{
	Super::NativeConstruct();

	auto ConfigureSlot = [](UInventorySlotWidget* SlotWidget, EInventorySlotType NewSlotType)
	{
		if (!SlotWidget) return;
		SlotWidget->SlotType = NewSlotType;
		SlotWidget->InteractionContext = EContextType::Sell;
	};

	ConfigureSlot(MainHand_Active, EInventorySlotType::MainHand);
	ConfigureSlot(OffHand_Active, EInventorySlotType::OffHand);
	ConfigureSlot(MainHand_0, EInventorySlotType::MainHand);
	ConfigureSlot(MainHand_1, EInventorySlotType::MainHand);
	ConfigureSlot(OffHand_0, EInventorySlotType::OffHand);
	ConfigureSlot(OffHand_1, EInventorySlotType::OffHand);
	ConfigureSlot(ArmorSlot, EInventorySlotType::Armor);
	ConfigureSlot(ConsumableSlot, EInventorySlotType::Consumable);

    ConfigureSlot(Ability_0, EInventorySlotType::Ability);
    ConfigureSlot(Ability_1, EInventorySlotType::Ability);
    ConfigureSlot(Ability_2, EInventorySlotType::Ability);

    UInventorySlotWidget* PerkWidgets[] = { Perk_0, Perk_1, Perk_2, Perk_3, Perk_4, Perk_5, Perk_6, Perk_7, Perk_8, Perk_9 };
    for (UInventorySlotWidget* PerkWidget : PerkWidgets)
    {
        ConfigureSlot(PerkWidget, EInventorySlotType::Perk);
    }

	UInventorySlotWidget* BagWidgets[] = { Inv_0, Inv_1, Inv_2, Inv_3, Inv_4, Inv_5, Inv_6, Inv_7, Inv_8, Inv_9, Inv_10, Inv_11 };
	for (UInventorySlotWidget* BagWidget : BagWidgets)
	{
		ConfigureSlot(BagWidget, EInventorySlotType::Bag);
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
