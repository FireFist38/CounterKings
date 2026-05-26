#include "ShopWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "ShopSlotWidget.h"
#include "CKGameMode.h"
#include "CKGameState.h"
#include "LootTableEntry.h"
#include "ItemBase.h"
#include "Kismet/GameplayStatics.h"

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (RerollButton) RerollButton->OnClicked.AddDynamic(this, &UShopWidget::OnRerollClicked);
    RefreshShopDisplay();
}

void UShopWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (GS && TimerText) TimerText->SetText(FText::AsNumber(FMath::CeilToInt(GS->GetRemainingTime())));

    APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (Character && Character->GetAttributeComponent() && GoldText)
        GoldText->SetText(FText::AsNumber(Character->GetAttributeComponent()->GetGold()));

    // Auto-refresh if data is stale (simple fix for visual sync)
    RefreshShopDisplay();
}

void UShopWidget::OnSlotClicked(int32 SlotIndex)
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (Character)
	{
		Character->Server_PurchaseItem(SlotIndex);
        FTimerHandle DelayTimer;
        GetWorld()->GetTimerManager().SetTimer(DelayTimer, this, &UShopWidget::RefreshShopDisplay, 0.25f, false);
	}
}

void UShopWidget::OnRerollClicked()
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (Character && Character->GetAttributeComponent())
	{
		Character->Server_RerollShop();
        FTimerHandle DelayTimer;
        GetWorld()->GetTimerManager().SetTimer(DelayTimer, this, &UShopWidget::RefreshShopDisplay, 0.25f, false);
	}
}

void UShopWidget::RefreshShopDisplay()
{
    APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (!Character || !Character->ShopDataTable) return;
    
    TArray<FName> Pool = Character->ShopPool;
    UShopSlotWidget* Slots[] = { ShopSlot_0, ShopSlot_1, ShopSlot_2, ShopSlot_3, ShopSlot_4 };

    for (int32 i = 0; i < 5; i++)
    {
        if (Slots[i])
        {
            AItemBase* DummyItem = nullptr;
            int32 Price = 50;
            
            if(Pool.IsValidIndex(i) && !Pool[i].IsNone()) 
            {
                FLootTableEntry* Entry = Character->ShopDataTable->FindRow<FLootTableEntry>(Pool[i], TEXT("ShopRefresh"));
                if (Entry && Entry->ItemClass)
                {
                    DummyItem = GetWorld()->SpawnActorDeferred<AItemBase>(Entry->ItemClass, FTransform::Identity);
                    if(DummyItem) DummyItem->FinishSpawning(FTransform::Identity);
                    Price = Entry->BuyPrice;
                }
            }
            
            Slots[i]->SetupSlot(DummyItem, i, Price);
            if(DummyItem) DummyItem->Destroy();
        }
    }
}
