#include "ShopContextMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "ItemBase.h"
#include "MainHandBase.h"
#include "SecondaryRangedBase.h"
#include "MagicWeaponBase.h"
#include "RangedWeaponBase.h"
#include "PlayerCharacter.h"
#include "ShieldBase.h"
#include "InventoryComponent.h"
#include "AttributeComponent.h"
#include "CKGameState.h"
#include "LootTableEntry.h"
#include "Kismet/GameplayStatics.h"

#include "SpellBeam.h"
#include "SpellProjectile.h"

void UShopContextMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ActionButton) ActionButton->OnClicked.AddDynamic(this, &UShopContextMenuWidget::OnActionClicked);
	if (CloseButton) CloseButton->OnClicked.AddDynamic(this, &UShopContextMenuWidget::OnCloseClicked);
}

void UShopContextMenuWidget::SetupContextMenu(AItemBase* Item, int32 SlotIndex, EContextType ContextType, ESlotGroup SourceGroup)
{
	CachedItem = Item;
	CachedSlotIndex = SlotIndex;
	CurrentType = ContextType;
	CachedSourceGroup = SourceGroup;
	
	if (Item)
	{
		ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
		FLootTableEntry* FoundEntry = nullptr;

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
			Table->GetAllRows<FLootTableEntry>(TEXT("ContextMenuLookup"), Rows);
			for (auto* Row : Rows)
			{
				if (Row->ItemClass && (Row->ItemClass == ItemClass || Row->ItemClass->GetPathName() == ItemClassName))
				{
					FoundEntry = Row;
					break;
				}
			}
			if (FoundEntry) break;
		}

        // Use Data Table name if found, fallback to Item class name
        if (FoundEntry && ItemNameText)
        {
            ItemNameText->SetText(FoundEntry->ItemName);
        }
        else if (ItemNameText)
        {
            ItemNameText->SetText(FText::FromName(Item->GetItemName()));
        }

        // --- UPDATED: Fallback to Item's Description if not in Loot Table ---
        if (FoundEntry && DescriptionText)
        {
            DescriptionText->SetText(FoundEntry->Description);
        }
        else if (DescriptionText)
        {
            DescriptionText->SetText(Item->GetDescription());
        }

		// Reset ranged/magic UI visibility each time (prevents stale state when widget is reused)
		if (RateOfFireText) RateOfFireText->SetVisibility(ESlateVisibility::Collapsed);
		if (MagazineCapacityText) MagazineCapacityText->SetVisibility(ESlateVisibility::Collapsed);
		if (PrimarySpellIcon) PrimarySpellIcon->SetVisibility(ESlateVisibility::Collapsed);
		if (SecondarySpellIcon) SecondarySpellIcon->SetVisibility(ESlateVisibility::Collapsed);

		if (DamageText)
		{
			APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
			UAttributeComponent* Attr = Character ? Character->GetAttributeComponent() : nullptr;

			auto FormatDamageBundle = [](const FDamageBundle& Dmg) -> FText
			{
				TArray<FString> Parts;
				auto AddIfNonZero = [&Parts](const FString& Label, float Value)
				{
					if (Value != 0.0f)
					{
						Parts.Add(FString::Printf(TEXT("%s: %d"), *Label, FMath::RoundToInt(Value)));
					}
				};

				AddIfNonZero(TEXT("Physical"), Dmg.Physical);
				AddIfNonZero(TEXT("Magic"), Dmg.Magic);
				AddIfNonZero(TEXT("Fire"), Dmg.Fire);
				AddIfNonZero(TEXT("Lightning"), Dmg.Lightning);
				AddIfNonZero(TEXT("Frost"), Dmg.Frost);
				AddIfNonZero(TEXT("Poison"), Dmg.Poison);
				AddIfNonZero(TEXT("Holy"), Dmg.Holy);
				AddIfNonZero(TEXT("Earth"), Dmg.Earth);

				if (Parts.Num() == 0)
				{
					return FText::FromString(TEXT("Damage: 0"));
				}

				return FText::FromString(FString::Join(Parts, TEXT(" | ")));
			};

			auto AppendLetterScalingLines = [](const EStatLetterScaling StrengthLetter, const EStatLetterScaling DexLetter, const EStatLetterScaling MagicLetter, const EStatLetterScaling LuckLetter, FString& OutDisplay)
			{
				auto AddIfSet = [&OutDisplay](const EStatLetterScaling Letter, const TCHAR* StatName)
				{
					if (Letter == EStatLetterScaling::Unspecified) return;

					const TCHAR* LetterChar = TEXT("?");
					switch (Letter)
					{
						case EStatLetterScaling::E: LetterChar = TEXT("E"); break;
						case EStatLetterScaling::D: LetterChar = TEXT("D"); break;
						case EStatLetterScaling::C: LetterChar = TEXT("C"); break;
						case EStatLetterScaling::B: LetterChar = TEXT("B"); break;
						case EStatLetterScaling::A: LetterChar = TEXT("A"); break;
						case EStatLetterScaling::S: LetterChar = TEXT("S"); break;
						default: LetterChar = TEXT("?"); break;
					}

					if (!OutDisplay.IsEmpty()) OutDisplay += TEXT("\n");
					OutDisplay += FString::Printf(TEXT("%s %s"), LetterChar, StatName);
				};

				AddIfSet(StrengthLetter, TEXT("Strength"));
				AddIfSet(DexLetter, TEXT("Dexterity"));
				AddIfSet(MagicLetter, TEXT("Magic"));
				AddIfSet(LuckLetter, TEXT("Luck"));
			};

			// Staffs / magic weapons
			if (AMagicWeaponBase* Staff = Cast<AMagicWeaponBase>(Item))
			{
				// Spell icons
				if (PrimarySpellIcon)
				{
					PrimarySpellIcon->SetVisibility(Staff->GetPrimarySpell() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
					if (USpellBase* PrimarySpell = Staff->GetPrimarySpell())
					{
						if (PrimarySpell->Icon)
						{
							PrimarySpellIcon->SetBrushFromTexture(PrimarySpell->Icon);
						}
					}
				}
				if (SecondarySpellIcon)
				{
					SecondarySpellIcon->SetVisibility(Staff->GetSecondarySpell() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
					if (USpellBase* SecondarySpell = Staff->GetSecondarySpell())
					{
						if (SecondarySpell->Icon)
						{
							SecondarySpellIcon->SetBrushFromTexture(SecondarySpell->Icon);
						}
					}
				}

				// Guns stat fields hidden for staffs
				if (RateOfFireText) RateOfFireText->SetVisibility(ESlateVisibility::Collapsed);
				if (MagazineCapacityText) MagazineCapacityText->SetVisibility(ESlateVisibility::Collapsed);

				const float StaffMult = Staff ? Staff->ComputeAttributeMultiplier(Attr) : 1.0f;

				auto ExtractSpellDamage = [](const USpellBase* Spell, const float Mult) -> FDamageBundle
				{
					FDamageBundle Out;
					if (!Spell) return Out;

					if (const USpellBeam* Beam = Cast<USpellBeam>(Spell))
					{
						Out = Beam->DamagePerTick;
						Out.Scale(Mult);
						return Out;
					}
					if (const USpellProjectile* Proj = Cast<USpellProjectile>(Spell))
					{
						Out = Proj->SpellDamage;
						Out.Scale(Mult);
						return Out;
					}
					return Out;
				};

				FDamageBundle PrimaryDmg = ExtractSpellDamage(Staff->GetPrimarySpell(), StaffMult);
				FDamageBundle SecondaryDmg = ExtractSpellDamage(Staff->GetSecondarySpell(), StaffMult);

				const bool bHasPrimary = PrimaryDmg.GetTotal() != 0.0f;
				const bool bHasSecondary = SecondaryDmg.GetTotal() != 0.0f;

				FString Display = TEXT("");
				if (bHasPrimary)
				{
					Display += FString::Printf(TEXT("Primary: %s"), *FormatDamageBundle(PrimaryDmg).ToString());
				}
				if (bHasSecondary)
				{
					if (!Display.IsEmpty()) Display += TEXT("\n");
					Display += FString::Printf(TEXT("Secondary: %s"), *FormatDamageBundle(SecondaryDmg).ToString());
				}

				// Append letter scaling lines (e.g., "S Strength", "B Magic")
				AppendLetterScalingLines(
					Staff->StrengthScalingLetter,
					Staff->DexterityScalingLetter,
					Staff->MagicScalingLetter,
					Staff->LuckScalingLetter,
					Display);

				if (Display.IsEmpty())
				{
					DamageText->SetText(FText::FromString(TEXT("Damage: -")));
					DamageText->SetVisibility(ESlateVisibility::Collapsed);
				}
				else
				{
					DamageText->SetText(FText::FromString(Display));
					DamageText->SetVisibility(ESlateVisibility::Visible);
				}
			}
			else
			{
				// Non-staff weapons (melee + ranged)
				// IMPORTANT: ARangedWeaponBase derives from AMainHandBase, so ranged must be checked BEFORE AMainHandBase.
				if (ARangedWeaponBase* RangedWeapon = Cast<ARangedWeaponBase>(Item))
				{
					// Main-hand ranged: show ROF + magazine capacity
					if (RateOfFireText)
					{
						const float RPM = RangedWeapon->GetFireRateRPM();
						RateOfFireText->SetText(FText::FromString(RPM > 0.0f
							? FString::Printf(TEXT("Fire Rate: %.2f RPM"), RPM)
							: TEXT("Fire Rate: -")));
						RateOfFireText->SetVisibility(ESlateVisibility::Visible);
					}

					if (MagazineCapacityText)
					{
						MagazineCapacityText->SetText(FText::FromString(FString::Printf(TEXT("Magazine: %d"), RangedWeapon->GetMagazineCapacity())));
						MagazineCapacityText->SetVisibility(ESlateVisibility::Visible);
					}

					FDamageBundle ScaledDamage = RangedWeapon->GetScaledDamage(Attr);
					FString Display = FormatDamageBundle(ScaledDamage).ToString();

					AppendLetterScalingLines(
						RangedWeapon->StrengthScalingLetter,
						RangedWeapon->DexterityScalingLetter,
						RangedWeapon->MagicScalingLetter,
						RangedWeapon->LuckScalingLetter,
						Display);

					DamageText->SetText(FText::FromString(Display));
					DamageText->SetVisibility(ESlateVisibility::Visible);
				}
				else if (AMainHandBase* Weapon = Cast<AMainHandBase>(Item))
				{
					// Melee: hide ranged stats fields
					if (RateOfFireText) RateOfFireText->SetVisibility(ESlateVisibility::Collapsed);
					if (MagazineCapacityText) MagazineCapacityText->SetVisibility(ESlateVisibility::Collapsed);

					FDamageBundle ScaledDamage = Weapon->GetScaledDamage(Attr);

					FString Display = FormatDamageBundle(ScaledDamage).ToString();
					AppendLetterScalingLines(
						Weapon->StrengthScalingLetter,
						Weapon->DexterityScalingLetter,
						Weapon->MagicScalingLetter,
						Weapon->LuckScalingLetter,
						Display);

					DamageText->SetText(FText::FromString(Display));
					DamageText->SetVisibility(ESlateVisibility::Visible);
				}
				else if (const ASecondaryRangedBase* Ranged = Cast<ASecondaryRangedBase>(Item))
				{
					// Offhand ranged: show ROF + magazine capacity + scaled damage
					if (RateOfFireText)
					{
						const float RPM = Ranged->GetFireRateRPM();
						RateOfFireText->SetText(FText::FromString(RPM > 0.0f
							? FString::Printf(TEXT("Fire Rate: %.2f RPM"), RPM)
							: TEXT("Fire Rate: -")));
						RateOfFireText->SetVisibility(ESlateVisibility::Visible);
					}

					if (MagazineCapacityText)
					{
						MagazineCapacityText->SetText(FText::FromString(FString::Printf(TEXT("Magazine: %d"), Ranged->GetMagazineCapacity())));
						MagazineCapacityText->SetVisibility(ESlateVisibility::Visible);
					}

					if (DamageText)
					{
						FDamageBundle ScaledDamage = Ranged->GetScaledDamageForUI(Attr);
						FString Display = FormatDamageBundle(ScaledDamage).ToString();

						AppendLetterScalingLines(
							Ranged->GetStrengthScalingLetter(),
							Ranged->GetDexterityScalingLetter(),
							Ranged->GetMagicScalingLetter(),
							Ranged->GetLuckScalingLetter(),
							Display);

						DamageText->SetText(FText::FromString(Display));
						DamageText->SetVisibility(ESlateVisibility::Visible);
					}
				}
				else if (AShieldBase* Shield = Cast<AShieldBase>(Item))
				{
					// Shields: show damage negation percentages
					if (RateOfFireText) RateOfFireText->SetVisibility(ESlateVisibility::Collapsed);
					if (MagazineCapacityText) MagazineCapacityText->SetVisibility(ESlateVisibility::Collapsed);

					if (DamageText)
					{
						// Shield negation values are authored as "flat" numbers (e.g. 7.0 for Physical),
						// not 0.07 percentages. Display them as-is to avoid inflated "700%" output.
						const float Physical = Shield->GetPhysicalNegation();
						const float Magic = Shield->GetMagicNegation();
						const float Fire = Shield->GetFireNegation();
						const float Lightning = Shield->GetLightningNegation();
						const float Frost = Shield->GetFrostNegation();
						const float Poison = Shield->GetPoisonNegation();
						const float Holy = Shield->GetHolyNegation();
						const float Earth = Shield->GetEarthNegation();

						FString Display;
						Display += TEXT("Negation:");
						auto AddNegationLine = [&Display](const FString& Label, float Value)
						{
							if (Value > 0.0f)
							{
								Display += FString::Printf(TEXT("\n%s %.1f%%"), *Label, Value * 100.0f);
							}
						};
						AddNegationLine(TEXT("Phys"), Physical);
						AddNegationLine(TEXT("Magic"), Magic);
						AddNegationLine(TEXT("Fire"), Fire);
						AddNegationLine(TEXT("Lightning"), Lightning);
						AddNegationLine(TEXT("Frost"), Frost);
						AddNegationLine(TEXT("Poison"), Poison);
						AddNegationLine(TEXT("Holy"), Holy);
						AddNegationLine(TEXT("Earth"), Earth);

						DamageText->SetText(FText::FromString(Display));
						DamageText->SetVisibility(ESlateVisibility::Visible);
					}
				}
				else
				{
					if (RateOfFireText) RateOfFireText->SetVisibility(ESlateVisibility::Collapsed);
					if (MagazineCapacityText) MagazineCapacityText->SetVisibility(ESlateVisibility::Collapsed);
					if (DamageText) DamageText->SetVisibility(ESlateVisibility::Collapsed);
				}
			}
		}

		// Safety: ensure offhand ranged ROF/mag/damage are visible when the item is ASecondaryRangedBase.
		// (Prevents situations where visibility gets left collapsed despite the offhand branch running.)
		if (const ASecondaryRangedBase* Ranged = Cast<ASecondaryRangedBase>(Item))
		{
			if (RateOfFireText) RateOfFireText->SetVisibility(ESlateVisibility::Visible);
			if (MagazineCapacityText) MagazineCapacityText->SetVisibility(ESlateVisibility::Visible);

			if (DamageText)
			{
				// If some edge case left it empty, show a minimal placeholder.
				if (DamageText->GetText().IsEmpty())
				{
					DamageText->SetText(FText::FromString(TEXT("Damage: -")));
				}
				DamageText->SetVisibility(ESlateVisibility::Visible);
			}
		}

        if (FoundEntry)
        {
            if (BuyPriceText) BuyPriceText->SetText(FText::Format(NSLOCTEXT("UI", "BuyLabel", "Buy: {0}"), FText::AsNumber(FoundEntry->BuyPrice)));
            if (SellPriceText) SellPriceText->SetText(FText::Format(NSLOCTEXT("UI", "SellLabel", "Sell: {0}"), FText::AsNumber(FoundEntry->SellPrice)));
        }
        else
        {
            if (BuyPriceText) BuyPriceText->SetText(FText::Format(NSLOCTEXT("UI", "BuyLabel", "Buy: {0}"), FText::AsNumber(Item->GetGoldValue() * 2)));
            if (SellPriceText) SellPriceText->SetText(FText::Format(NSLOCTEXT("UI", "SellLabel", "Sell: {0}"), FText::AsNumber(Item->GetGoldValue())));
        }
		
		FString ActionLabel = TEXT("Action");
		switch (ContextType)
		{
			case EContextType::Purchase: ActionLabel = TEXT("Purchase"); break;
			case EContextType::Sell:     ActionLabel = TEXT("Sell");     break;
			case EContextType::Equip:    
                ActionLabel = (SourceGroup == ESlotGroup::Bag) ? TEXT("Equip") : TEXT("Unequip"); 
                break;
			case EContextType::Drop:     ActionLabel = TEXT("Drop");     break;
		}
		if (ActionButtonText) ActionButtonText->SetText(FText::FromString(ActionLabel));
	}
}

void UShopContextMenuWidget::OnActionClicked()
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (!Character || !Character->GetInventoryComponent()) 
    {
        return;
    }

	switch (CurrentType)
	{
		case EContextType::Purchase:
			Character->Server_PurchaseItem(CachedSlotIndex);
			break;
		case EContextType::Sell:
            Character->Server_SellItem_Direct(CachedSlotIndex);
			break;
		case EContextType::Drop:
			if (CachedItem) Character->GetInventoryComponent()->Server_DropItem(CachedSourceGroup, CachedSlotIndex);
			break;
		case EContextType::Equip:
            if (CachedSourceGroup == ESlotGroup::Bag)
            {
                Character->GetInventoryComponent()->Server_MoveItem(CachedSourceGroup, CachedSlotIndex, ESlotGroup::MainHand, 0);
            }
            else
            {
                for(int i = 0; i < 12; i++)
                {
                    if (!Character->GetInventoryComponent()->GetBagItem(i))
                    {
                        Character->GetInventoryComponent()->Server_MoveItem(CachedSourceGroup, CachedSlotIndex, ESlotGroup::Bag, i);
                        break;
                    }
                }
            }
			break;
	}
	RemoveFromParent();
}

void UShopContextMenuWidget::OnCloseClicked()
{
	RemoveFromParent();
}
