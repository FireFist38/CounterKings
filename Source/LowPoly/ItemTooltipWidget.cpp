#include "ItemTooltipWidget.h"
#include "Components/TextBlock.h"
#include "ItemBase.h"
#include "CKGameState.h"
#include "LootTableEntry.h"
#include "Kismet/GameplayStatics.h"

void UItemTooltipWidget::SetTooltipData(AItemBase* Item)
{
	if (!Item) return;

	ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!GS) return;

	TArray<UDataTable*> SearchTables = GS->GetAllItemTables();

	FString ItemClassName = Item->GetClass()->GetPathName();
	UClass* ItemClass = Item->GetClass();

	FLootTableEntry* FoundEntry = nullptr;
	for (UDataTable* Table : SearchTables)
	{
		if (!Table) continue;

		TArray<FLootTableEntry*> Rows;
		Table->GetAllRows<FLootTableEntry>(TEXT("TooltipLookup"), Rows);
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

	// Data tables are the single source of truth. Leave blank if row is missing.
	if (FoundEntry)
	{
		if (NameText) NameText->SetText(FoundEntry->ItemName);
		if (DescriptionText) DescriptionText->SetText(FoundEntry->Description);
	}
}