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

void UPreRoundRootWidget::ShowShop()
{
	if (ContentSwitcher) ContentSwitcher->SetActiveWidgetIndex(1); // Assuming 0 is Menu, 1 is Shop
	if (BackButton) BackButton->SetVisibility(ESlateVisibility::Visible);
}

void UPreRoundRootWidget::ShowLevelUp()
{
	if (ContentSwitcher) ContentSwitcher->SetActiveWidgetIndex(2); // Assuming 2 is LevelUp
	if (BackButton) BackButton->SetVisibility(ESlateVisibility::Visible);
}

void UPreRoundRootWidget::ShowEquip()
{
	if (ContentSwitcher) ContentSwitcher->SetActiveWidgetIndex(3); // Assuming 3 is Equip
	if (BackButton) BackButton->SetVisibility(ESlateVisibility::Visible);
}

void UPreRoundRootWidget::GoBackToMenu()
{
	if (ContentSwitcher) ContentSwitcher->SetActiveWidgetIndex(0); // Return to Root
	if (BackButton) BackButton->SetVisibility(ESlateVisibility::Collapsed);
}

void UPreRoundRootWidget::ReadyUp()
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (Character)
	{
		Character->Server_SetReadyForNextRound(true);
	}
}
