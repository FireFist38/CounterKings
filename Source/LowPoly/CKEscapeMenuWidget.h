#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKEscapeMenuWidget.generated.h"

class UButton;
class UCKSettingsWidget;

UCLASS(Abstract)
class LOWPOLY_API UCKEscapeMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* ResumeButton;

	UPROPERTY(meta = (BindWidget))
	UButton* SettingsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitToTitleButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitToDesktopButton;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UButton* DebugWinButton;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UButton* DebugLoseButton;

	UPROPERTY(EditDefaultsOnly, Category = "CK|Menu")
	TSubclassOf<UCKSettingsWidget> SettingsWidgetClass;

	// Level name to open when quitting to title (e.g. "MainMenu")
	UPROPERTY(EditDefaultsOnly, Category = "CK|Menu")
	FName TitleLevelName = FName("MainMenu");

	UPROPERTY()
	UCKSettingsWidget* SettingsWidgetInstance;

	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnQuitToTitleClicked();

	UFUNCTION()
	void OnQuitToDesktopClicked();

	UFUNCTION()
	void OnDebugWinClicked();

	UFUNCTION()
	void OnDebugLoseClicked();

public:
	void CloseMenu();
};
