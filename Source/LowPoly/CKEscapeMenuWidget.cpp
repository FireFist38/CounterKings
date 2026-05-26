#include "CKEscapeMenuWidget.h"
#include "PlayerCharacter.h"
#include "CKSettingsWidget.h"
#include "CKGameUserSettings.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UCKEscapeMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResumeButton)
		ResumeButton->OnClicked.AddDynamic(this, &UCKEscapeMenuWidget::OnResumeClicked);
	if (SettingsButton)
		SettingsButton->OnClicked.AddDynamic(this, &UCKEscapeMenuWidget::OnSettingsClicked);
	if (QuitToTitleButton)
		QuitToTitleButton->OnClicked.AddDynamic(this, &UCKEscapeMenuWidget::OnQuitToTitleClicked);
	if (QuitToDesktopButton)
		QuitToDesktopButton->OnClicked.AddDynamic(this, &UCKEscapeMenuWidget::OnQuitToDesktopClicked);
	if (DebugWinButton)
		DebugWinButton->OnClicked.AddDynamic(this, &UCKEscapeMenuWidget::OnDebugWinClicked);
	if (DebugLoseButton)
		DebugLoseButton->OnClicked.AddDynamic(this, &UCKEscapeMenuWidget::OnDebugLoseClicked);
}

void UCKEscapeMenuWidget::OnResumeClicked()
{
	CloseMenu();
}

void UCKEscapeMenuWidget::OnSettingsClicked()
{
	if (!SettingsWidgetClass) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	if (SettingsWidgetInstance && SettingsWidgetInstance->IsInViewport())
		return;

	SettingsWidgetInstance = CreateWidget<UCKSettingsWidget>(PC, SettingsWidgetClass);
	if (SettingsWidgetInstance)
		SettingsWidgetInstance->AddToViewport(10);
}

void UCKEscapeMenuWidget::OnQuitToTitleClicked()
{
	UGameplayStatics::OpenLevel(this, TitleLevelName);
}

void UCKEscapeMenuWidget::OnQuitToDesktopClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UCKEscapeMenuWidget::OnDebugWinClicked()
{
	if (APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
	{
		PC->Server_DebugWinRound();
		CloseMenu();
	}
}

void UCKEscapeMenuWidget::OnDebugLoseClicked()
{
	if (APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
	{
		PC->Server_DebugLoseRound();
		CloseMenu();
	}
}

void UCKEscapeMenuWidget::CloseMenu()
{
	if (SettingsWidgetInstance && SettingsWidgetInstance->IsInViewport())
		SettingsWidgetInstance->RemoveFromParent();

	if (UCKGameUserSettings* Settings = UCKGameUserSettings::GetCKSettings())
		Settings->SaveCKSettings();

	RemoveFromParent();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
}
