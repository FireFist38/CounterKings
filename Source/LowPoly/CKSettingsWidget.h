#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKSettingsWidget.generated.h"

class USlider;
class UTextBlock;
class UButton;
class UWidget;

UENUM(BlueprintType)
enum class ESettingsTab : uint8
{
	Video,
	Audio,
	Controls,
	Accessibility
};

UCLASS(Abstract)
class LOWPOLY_API UCKSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// --- Tab buttons ---
	UPROPERTY(meta = (BindWidget))
	UButton* VideoButton;

	UPROPERTY(meta = (BindWidget))
	UButton* AudioButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ControlsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* AccessibilityButton;

	// --- Tab panels (any widget, usually a VerticalBox or similar container) ---
	UPROPERTY(meta = (BindWidget))
	UWidget* VideoPanel;

	UPROPERTY(meta = (BindWidget))
	UWidget* AudioPanel;

	UPROPERTY(meta = (BindWidget))
	UWidget* ControlsPanel;

	UPROPERTY(meta = (BindWidget))
	UWidget* AccessibilityPanel;

	// --- Controls tab: sensitivity (optional — only bind if widgets are placed in the Controls panel) ---
	UPROPERTY(meta = (BindWidgetOptional))
	USlider* SensitivitySlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SensitivityValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	USlider* AimSensitivitySlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* AimSensitivityValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* CloseButton;

	UPROPERTY(EditDefaultsOnly, Category = "CK|Settings")
	ESettingsTab DefaultTab = ESettingsTab::Controls;

	UPROPERTY(EditDefaultsOnly, Category = "CK|Settings")
	float MinSensitivity = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "CK|Settings")
	float MaxSensitivity = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "CK|Settings")
	float MinAimMultiplier = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "CK|Settings")
	float MaxAimMultiplier = 1.5f;

	UFUNCTION()
	void OnVideoTabClicked();

	UFUNCTION()
	void OnAudioTabClicked();

	UFUNCTION()
	void OnControlsTabClicked();

	UFUNCTION()
	void OnAccessibilityTabClicked();

	UFUNCTION()
	void OnSensitivityChanged(float NewValue);

	UFUNCTION()
	void OnAimSensitivityChanged(float NewValue);

	UFUNCTION()
	void OnCloseClicked();

	void ShowTab(ESettingsTab Tab);

	float ValueToSlider(float Value, float Min, float Max) const;
	float SliderToValue(float SliderValue, float Min, float Max) const;
};
