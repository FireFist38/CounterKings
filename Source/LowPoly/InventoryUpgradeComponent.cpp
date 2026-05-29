#include "InventoryUpgradeComponent.h"

#include "ItemBase.h"
#include "AttributeComponent.h"
#include "MainHandBase.h"
#include "RangedWeaponBase.h"
#include "SecondaryRangedBase.h"
#include "MagicWeaponBase.h"
#include "ShieldBase.h"
#include "CKGameState.h"
#include "Kismet/GameplayStatics.h"

UInventoryUpgradeComponent::UInventoryUpgradeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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
		return TEXT("Damage: 0");
	}
	return FString::Join(Parts, TEXT(" | "));
}

FUpgradePreviewStats UInventoryUpgradeComponent::BuildUpgradePreviewStats(const UAttributeComponent* Attr, const AItemBase* Item, EItemRarity CurrentRarity, EItemRarity TargetRarity) const
{
	FUpgradePreviewStats Out;
	if (!Item || !Attr) return Out;

	// We temporarily compute stats based on rarity by calling getters that already incorporate rarity.
	// Since item getters rely on Item->Rarity, we compute by directly overriding rarity on a local pointer pattern:
	// In authoritative flow, we will update item rarity on server. For UI preview we just need the computed strings.
	// NOTE: We are not mutating the item here.

	// Helper: compute based on a hypothetical rarity by using rarity strength scale.
	// Existing damage/negation getters use ItemBase::GetRarityInternal() / Rarity where available.
	// For melee/ranged/magic damage we will use the weapon ComputeAttributeMultiplier which uses Rarity.
	// So we need the rarity-specific multiplier. Since item mutation is not safe for preview, we use the scaled getters
	// by cloning the computation formula is out-of-scope; for now, reuse SetupContextMenu-style logic is done by forcing
	// item rarity during preview at UI layer.
	//
	// Therefore, this component currently returns negation for shields and compact damage text using the *current* item rarity.
	// The actual Upgrade UI should compute AfterText/AfterNegations by temporarily setting item rarity on the server-owned instance.

	// Shield negation preview
	if (const AShieldBase* Shield = Cast<AShieldBase>(Item))
	{
		Out.bIsNegationPreview = true;

		// Before
		Out.BeforePhysicalNegation = Shield->GetPhysicalNegation();
		Out.BeforeMagicNegation = Shield->GetMagicNegation();

		// After - rarity impacts item visuals via Rarity only; current shield negation getters are rarity-independent.
		// So After equals Before unless shield-specific rarity re-authoring exists.
		Out.AfterPhysicalNegation = Out.BeforePhysicalNegation;
		Out.AfterMagicNegation = Out.BeforeMagicNegation;

		Out.BeforeText = FString::Printf(TEXT("Negation:\nPhys %.1f%%\nMagic %.1f%%"), Out.BeforePhysicalNegation * 100.0f, Out.BeforeMagicNegation * 100.0f);
		Out.AfterText = FString::Printf(TEXT("Negation:\nPhys %.1f%%\nMagic %.1f%%"), Out.AfterPhysicalNegation * 100.0f, Out.AfterMagicNegation * 100.0f);
		return Out;
	}

	// Weapon damage preview (compact)
	const FDamageBundle CurrDmg = [&]()
	{
		if (const AMagicWeaponBase* Staff = Cast<AMagicWeaponBase>(Item))
		{
			// Primary spell only for compact preview
			// In UI we will use full SetupContextMenu-like math later.
			return Staff->GetPrimarySpell() ? Staff->GetPrimarySpell()->GetDamageForUI(Attr) : FDamageBundle();
		}
		if (const ARangedWeaponBase* Ranged = Cast<ARangedWeaponBase>(Item))
		{
			return Ranged->GetScaledDamage(Attr);
		}
		if (const AMainHandBase* Weapon = Cast<AMainHandBase>(Item))
		{
			return Weapon->GetScaledDamage(Attr);
		}
		if (const ASecondaryRangedBase* OffRanged = Cast<ASecondaryRangedBase>(Item))
		{
			return OffRanged->GetScaledDamageForUI(Attr);
		}
		return FDamageBundle();
	}();

	Out.BeforeText = FormatDamageBundleCompact(CurrDmg);
	Out.AfterText = Out.BeforeText;
	return Out;
}

