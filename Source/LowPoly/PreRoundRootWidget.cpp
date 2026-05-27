#include "PreRoundRootWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "CKGameMode.h"
#include "CKGameState.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void UPreRoundRootWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (NavShopButton) NavShopButton->OnClicked.AddDynamic(this, &UPreRoundRootWidget::ShowShop);
	if (NavLevelUpButton) NavLevelUpButton->OnClicked.AddDynamic(this, &UPreRoundRootWidget::ShowLevelUp);
	if (NavEquipButton) NavEquipButton->OnClicked.AddDynamic(this, &UPreRoundRootWidget::ShowEquip);
	if (BackButton) BackButton->OnClicked.AddDynamic(this, &UPreRoundRootWidget::GoBackToMenu);
	if (ReadyUpButton) ReadyUpButton->OnClicked.AddDynamic(this, &UPreRoundRootWidget::ReadyUp);

	// Ensure BackButton starts hidden
	if (BackButton) BackButton->SetVisibility(ESlateVisibility::Collapsed);
}

void UPreRoundRootWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (GS && TimerText)
	{
		TimerText->SetText(FText::AsNumber(FMath::CeilToInt(GS->GetRemainingTime())));
	}
}

void UPreRoundRootWidget::SwitchToTab(int32 Index)
{
    if (ContentSwitcher)
    {
        ContentSwitcher->SetActiveWidgetIndex(Index);
        if (BackButton)
        {
            BackButton->SetVisibility(Index == 0 ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
        }
    }
}

void UPreRoundRootWidget::ShowShop()
{
    SwitchToTab(1);
}

void UPreRoundRootWidget::ShowLevelUp()
{
    SwitchToTab(2);
}

void UPreRoundRootWidget::ShowEquip()
{
    SwitchToTab(3);
}

void UPreRoundRootWidget::GoBackToMenu()
{
    SwitchToTab(0);
}

void UPreRoundRootWidget::ReadyUp()
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (Character)
	{
		Character->Server_SetReadyForNextRound(true);
	}
}
