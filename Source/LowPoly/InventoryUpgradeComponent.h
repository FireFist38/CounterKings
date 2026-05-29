#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemBase.h"
#include "EquipmentUpgradeTypes.h"
#include "Engine/DataTable.h"
#include "InventoryUpgradeComponent.generated.h"

class AItemBase;
class UAttributeComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LOWPOLY_API UInventoryUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryUpgradeComponent();

	// Computes upgrade cost using a data table.
	// Returns true if a matching row was found.
	bool GetUpgradeCostFor(EItemRarity CurrentRarity, EItemRarity TargetRarity, int32& OutCost) const;

	// Returns upgrade target rarity (currently only supports Next or ToLegendary).
	EItemRarity ResolveTargetRarity(EItemRarity CurrentRarity, EItemUpgradeTarget Target) const;

	// Preview helpers: compute negation preview for shields and damage preview for weapons using existing getter logic.
	// This only builds strings/values for UI; authoritative changes happen via server RPC elsewhere.
	FUpgradePreviewStats BuildUpgradePreviewStats(const UAttributeComponent* Attr, const AItemBase* Item, EItemRarity CurrentRarity, EItemRarity TargetRarity) const;

	// Editable in editor/blueprint: data table that maps CurrentRarity + NextRarity to cost.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CK|Upgrade")
	class UDataTable* UpgradeCostTable;
};

