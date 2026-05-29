#include "InventoryUpgradeComponent.h"

#include "ItemBase.h"
#include "AttributeComponent.h"
#include "MainHandBase.h"
#include "RangedWeaponBase.h"
#include "SecondaryRangedBase.h"
#include "MagicWeaponBase.h"
#include "ShieldBase.h"
#include "ArmorBase.h"
#include "SpellBase.h"
#include "SpellProjectile.h"
#include "CKGameState.h"
#include "Kismet/GameplayStatics.h"
#include "ItemUpgradeCost.h"

UInventoryUpgradeComponent::UInventoryUpgradeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	UpgradeCostTable = nullptr;
}

EItemRarity UInventoryUpgradeComponent::ResolveTargetRarity(EItemRarity CurrentRarity, EItemUpgradeTarget Target) const
{
	switch (Target)
	{
	case EItemUpgradeTarget::Next:
		{
			// Common -> Uncommon -> Rare -> Legendary
			switch (CurrentRarity)
			{
			case EItemRarity::Common: return EItemRarity::Uncommon;
			case EItemRarity::Uncommon: return EItemRarity::Rare;
			case EItemRarity::Rare: return EItemRarity::Legendary;
			case EItemRarity::Legendary: return EItemRarity::Legendary;
			default: return EItemRarity::Legendary;
			}
		}
	case EItemUpgradeTarget::ToLegendary:
		return EItemRarity::Legendary;
	default:
		return EItemRarity::Legendary;
	}
}

bool UInventoryUpgradeComponent::GetUpgradeCostFor(EItemRarity CurrentRarity, EItemRarity TargetRarity, int32& OutCost) const
{
	OutCost = 0;
	if (!UpgradeCostTable) return false;

	// Table rows are expected to be FItemUpgradeCostRow.
	TArray<FItemUpgradeCostRow*> Rows;
	UpgradeCostTable->GetAllRows<FItemUpgradeCostRow>(TEXT("UpgradeCostLookup"), Rows);
	if (Rows.Num() == 0) return false;

	for (FItemUpgradeCostRow* Row : Rows)
	{
		if (!Row) continue;
		if (Row->CurrentRarity == CurrentRarity && Row->NextRarity == TargetRarity)
		{
			OutCost = Row->GoldCost;
			return true;
		}
	}

	return false;
}

static FString FormatDamageBundleCompact(const FDamageBundle& Dmg)
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
		return TEXT("0");
	}
	return FString::Join(Parts, TEXT(" | "));
}

FUpgradePreviewStats UInventoryUpgradeComponent::BuildUpgradePreviewStats(const UAttributeComponent* Attr, const AItemBase* Item, EItemRarity CurrentRarity, EItemRarity TargetRarity) const
{
	FUpgradePreviewStats Out;
	if (!Item || !Attr) return Out;

    TArray<FString> BeforeLines;
    TArray<FString> AfterLines;

    auto AddStatLine = [&](const FString& Label, const FString& BeforeVal, const FString& AfterVal)
    {
        BeforeLines.Add(FString::Printf(TEXT("%s: %s"), *Label, *BeforeVal));
        AfterLines.Add(FString::Printf(TEXT("%s: %s"), *Label, *AfterVal));
    };

	// 0. Spell Name (for Staves/Magic items)
	if (const AMagicWeaponBase* Staff = Cast<AMagicWeaponBase>(Item))
	{
		if (Staff->GetPrimarySpell())
		{
			FString SpellName = Staff->GetPrimarySpell()->DisplayName.ToString();
			if (SpellName.IsEmpty() || SpellName == TEXT("Unnamed Spell")) SpellName = TEXT("Active Spell");
			AddStatLine(TEXT("Spell"), SpellName, SpellName);
		}
	}

	// 1. Damage (for Weapons/Offhands)
	if (Item->GetItemType() == EItemType::Melee || Item->GetItemType() == EItemType::Ranged || Item->GetItemType() == EItemType::Magic)
	{
        auto GetDmgText = [&](EItemRarity RarityToPreview) -> FString
        {
            FDamageBundle FinalDmg;
            if (const AMagicWeaponBase* Staff = Cast<AMagicWeaponBase>(Item))
            {
                // Magic Total = (Spell Base + Staff Bonus) * Attr
                if (Staff->GetPrimarySpell())
                {
                    if (const USpellProjectile* Proj = Cast<USpellProjectile>(Staff->GetPrimarySpell()))
                    {
                        FinalDmg = Proj->SpellDamage;
                        FDamageBundle StaffBonus = Staff->GetDamageForRarity(RarityToPreview);
                        FinalDmg.Physical += StaffBonus.Physical;
                        FinalDmg.Magic += StaffBonus.Magic;
                        FinalDmg.Fire += StaffBonus.Fire;
                        FinalDmg.Lightning += StaffBonus.Lightning;
                        FinalDmg.Frost += StaffBonus.Frost;
                        FinalDmg.Poison += StaffBonus.Poison;
                        FinalDmg.Holy += StaffBonus.Holy;
                        FinalDmg.Earth += StaffBonus.Earth;
                        
                        const float RarityScale = AItemBase::GetRarityStrengthScale(RarityToPreview);
                        const float MagicCoeff = AItemBase::GetLetterStatScalingCoeff(Staff->MagicScalingLetter);
                        float Mult = 1.0f + RarityScale * (Attr->Magic * MagicCoeff);
                        FinalDmg.Scale(Mult);
                    }
                }
            }
            else if (const AMainHandBase* MH = Cast<AMainHandBase>(Item))
            {
                FinalDmg = Item->GetDamageForRarity(RarityToPreview);
                const float RarityScale = AItemBase::GetRarityStrengthScale(RarityToPreview);
                const float StrengthCoeff  = AItemBase::GetLetterStatScalingCoeff(MH->StrengthScalingLetter);
                const float DexterityCoeff = AItemBase::GetLetterStatScalingCoeff(MH->DexterityScalingLetter);
                const float MagicCoeff     = AItemBase::GetLetterStatScalingCoeff(MH->MagicScalingLetter);
                const float LuckCoeff      = AItemBase::GetLetterStatScalingCoeff(MH->LuckScalingLetter);
                float Mult = 1.0f + RarityScale * (Attr->Strength * StrengthCoeff + Attr->Dexterity * DexterityCoeff + Attr->Magic * MagicCoeff + Attr->Luck * LuckCoeff);
                FinalDmg.Scale(Mult);
            }
            else if (const ASecondaryRangedBase* SR = Cast<ASecondaryRangedBase>(Item))
            {
                FinalDmg = Item->GetDamageForRarity(RarityToPreview);
                const float RarityScale = AItemBase::GetRarityStrengthScale(RarityToPreview);
                const float StrengthCoeff  = AItemBase::GetLetterStatScalingCoeff(SR->GetStrengthScalingLetter());
                const float DexterityCoeff = AItemBase::GetLetterStatScalingCoeff(SR->GetDexterityScalingLetter());
                const float MagicCoeff     = AItemBase::GetLetterStatScalingCoeff(SR->GetMagicScalingLetter());
                const float LuckCoeff      = AItemBase::GetLetterStatScalingCoeff(SR->GetLuckScalingLetter());
                float Mult = 1.0f + RarityScale * (Attr->Strength * StrengthCoeff + Attr->Dexterity * DexterityCoeff + Attr->Magic * MagicCoeff + Attr->Luck * LuckCoeff);
                FinalDmg.Scale(Mult);
            }
            return FormatDamageBundleCompact(FinalDmg);
        };

        AddStatLine(TEXT("Damage"), GetDmgText(CurrentRarity), GetDmgText(TargetRarity));
	}

    // 2. Ranged Performance
    if (const ARangedWeaponBase* Ranged = Cast<ARangedWeaponBase>(Item))
    {
        // Magazine
        int32 MagBefore = Ranged->GetMagazineCapacityForRarity(CurrentRarity);
        if (MagBefore <= 0) MagBefore = Ranged->GetMagazineCapacity();
        int32 MagAfter = Ranged->GetMagazineCapacityForRarity(TargetRarity);
        if (MagAfter <= 0) MagAfter = MagBefore;
        if (MagBefore != MagAfter || MagAfter > 1) // Only show if relevant
            AddStatLine(TEXT("Magazine"), FString::FromInt(MagBefore), FString::FromInt(MagAfter));

        // Fire Rate
        float RPMBefore = Ranged->GetFireRateRPMForRarity(CurrentRarity);
        if (RPMBefore <= 0.0f) RPMBefore = Ranged->GetFireRateRPM();
        float RPMAfter = Ranged->GetFireRateRPMForRarity(TargetRarity);
        if (RPMAfter <= 0.0f) RPMAfter = RPMBefore;
        if (RPMBefore != RPMAfter || RPMAfter > 0.0f)
            AddStatLine(TEXT("RPM"), FString::Printf(TEXT("%.0f"), RPMBefore), FString::Printf(TEXT("%.0f"), RPMAfter));

        // Reload
        float ReloadBefore = Ranged->GetReloadTimeForRarity(CurrentRarity);
        if (ReloadBefore <= 0.0f) ReloadBefore = Ranged->GetReloadTime();
        float ReloadAfter = Ranged->GetReloadTimeForRarity(TargetRarity);
        if (ReloadAfter <= 0.0f) ReloadAfter = ReloadBefore;
        if (ReloadBefore != ReloadAfter)
            AddStatLine(TEXT("Reload Time"), FString::Printf(TEXT("%.1fs"), ReloadBefore), FString::Printf(TEXT("%.1fs"), ReloadAfter));
    }

    // 3. Resource Costs
    if (const AMainHandBase* MH = Cast<AMainHandBase>(Item))
    {
        float ManaBefore = MH->GetManaCostForRarity(CurrentRarity);
        if (ManaBefore <= 0.0f) ManaBefore = MH->GetManaCost();
        float ManaAfter = MH->GetManaCostForRarity(TargetRarity);
        if (ManaAfter <= 0.0f) ManaAfter = ManaBefore;
        if (ManaBefore != ManaAfter && ManaAfter > 0.0f)
            AddStatLine(TEXT("Mana Cost"), FString::Printf(TEXT("%.0f"), ManaBefore), FString::Printf(TEXT("%.0f"), ManaAfter));

        float StamBefore = MH->GetStaminaCostForRarity(CurrentRarity);
        if (StamBefore <= 0.0f) StamBefore = MH->GetStaminaCost();
        float StamAfter = MH->GetStaminaCostForRarity(TargetRarity);
        if (StamAfter <= 0.0f) StamAfter = StamBefore;
        if (StamBefore != StamAfter && StamAfter > 0.0f)
            AddStatLine(TEXT("Stamina Cost"), FString::Printf(TEXT("%.0f"), StamBefore), FString::Printf(TEXT("%.0f"), StamAfter));
    }

    // 4. Defense (Shields)
    if (const AShieldBase* Shield = Cast<AShieldBase>(Item))
    {
        Out.bIsNegationPreview = true;
        float PhysBefore = Shield->GetPhysicalNegationForRarity(CurrentRarity);
        float PhysAfter = Shield->GetPhysicalNegationForRarity(TargetRarity);
        AddStatLine(TEXT("Physical Block"), FString::Printf(TEXT("%.0f%%"), PhysBefore * 100.0f), FString::Printf(TEXT("%.0f%%"), PhysAfter * 100.0f));

        float MagBefore = Shield->GetMagicNegationForRarity(CurrentRarity);
        float MagAfter = Shield->GetMagicNegationForRarity(TargetRarity);
        if (MagBefore > 0.0f || MagAfter > 0.0f)
            AddStatLine(TEXT("Magic Block"), FString::Printf(TEXT("%.0f%%"), MagBefore * 100.0f), FString::Printf(TEXT("%.0f%%"), MagAfter * 100.0f));
    }

    // 5. Armor
    if (const AArmorBase* Armor = Cast<AArmorBase>(Item))
    {
        float HPBefore = Armor->GetHealthBonusForRarity(CurrentRarity);
        float HPAfter = Armor->GetHealthBonusForRarity(TargetRarity);
        if (HPBefore != HPAfter || HPAfter > 0.0f)
            AddStatLine(TEXT("HP Bonus"), FString::Printf(TEXT("%d"), FMath::RoundToInt(HPBefore)), FString::Printf(TEXT("%d"), FMath::RoundToInt(HPAfter)));

        float SpeedBefore = Armor->GetMovementSpeedModifierForRarity(CurrentRarity);
        float SpeedAfter = Armor->GetMovementSpeedModifierForRarity(TargetRarity);
        if (SpeedBefore != SpeedAfter || SpeedAfter != 1.0f)
            AddStatLine(TEXT("Speed Multiplier"), FString::Printf(TEXT("x%.2f"), SpeedBefore), FString::Printf(TEXT("x%.2f"), SpeedAfter));
    }

    Out.BeforeText = FString::Join(BeforeLines, TEXT("\n"));
    Out.AfterText = FString::Join(AfterLines, TEXT("\n"));

	return Out;
}
