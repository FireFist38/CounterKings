#include "CKSettingsWidget.h"
#include "CKGameUserSettings.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Widget.h"

void UCKSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Tab button bindings
	if (VideoButton)
		VideoButton->OnClicked.AddDynamic(this, &UCKSettingsWidget::OnVideoTabClicked);
	if (AudioButton)
		AudioButton->OnClicked.AddDynamic(this, &UCKSettingsWidget::OnAudioTabClicked);
	if (ControlsButton)
		ControlsButton->OnClicked.AddDynamic(this, &UCKSettingsWidget::OnControlsTabClicked);
	if (AccessibilityButton)
		AccessibilityButton->OnClicked.AddDynamic(this, &UCKSettingsWidget::OnAccessibilityTabClicked);

	// Sensitivity widgets (Controls tab) — bind regardless so changes work even if Settings is null
	UCKGameUserSettings* Settings = UCKGameUserSettings::GetCKSettings();

	if (SensitivitySlider)
	{
		if (Settings)
			SensitivitySlider->SetValue(ValueToSlider(Settings->GetMouseSensitivity(), MinSensitivity, MaxSensitivity));
		SensitivitySlider->OnValueChanged.AddDynamic(this, &UCKSettingsWidget::OnSensitivityChanged);
	}
	if (SensitivityValueText && Settings)
		SensitivityValueText->SetText(FText::AsNumber(FMath::RoundToFloat(Settings->GetMouseSensitivity() * 100.0f) / 100.0f));

	if (AimSensitivitySlider)
	{
		if (Settings)
			AimSensitivitySlider->SetValue(ValueToSlider(Settings->GetAimSensitivityMultiplier(), MinAimMultiplier, MaxAimMultiplier));
		AimSensitivitySlider->OnValueChanged.AddDynamic(this, &UCKSettingsWidget::OnAimSensitivityChanged);
	}
	if (AimSensitivityValueText && Settings)
		AimSensitivityValueText->SetText(FText::AsNumber(FMath::RoundToFloat(Settings->GetAimSensitivityMultiplier() * 100.0f) / 100.0f));

	if (CloseButton)
		CloseButton->OnClicked.AddDynamic(this, &UCKSettingsWidget::OnCloseClicked);

	ShowTab(DefaultTab);
}

void UCKSettingsWidget::ShowTab(ESettingsTab Tab)
{
	if (VideoPanel)
		VideoPanel->SetVisibility(Tab == ESettingsTab::Video ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (AudioPanel)
		AudioPanel->SetVisibility(Tab == ESettingsTab::Audio ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (ControlsPanel)
		ControlsPanel->SetVisibility(Tab == ESettingsTab::Controls ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (AccessibilityPanel)
		AccessibilityPanel->SetVisibility(Tab == ESettingsTab::Accessibility ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UCKSettingsWidget::OnVideoTabClicked() { ShowTab(ESettingsTab::Video); }
void UCKSettingsWidget::OnAudioTabClicked() { ShowTab(ESettingsTab::Audio); }
void UCKSettingsWidget::OnControlsTabClicked() { ShowTab(ESettingsTab::Controls); }
void UCKSettingsWidget::OnAccessibilityTabClicked() { ShowTab(ESettingsTab::Accessibility); }

void UCKSettingsWidget::OnSensitivityChanged(float NewValue)
{
	UCKGameUserSettings* Settings = UCKGameUserSettings::GetCKSettings();
	if (!Settings) return;

	const float Value = SliderToValue(NewValue, MinSensitivity, MaxSensitivity);
	Settings->SetMouseSensitivity(Value);

	if (SensitivityValueText)
		SensitivityValueText->SetText(FText::AsNumber(FMath::RoundToFloat(Value * 100.0f) / 100.0f));
}

void UCKSettingsWidget::OnAimSensitivityChanged(float NewValue)
{
	UCKGameUserSettings* Settings = UCKGameUserSettings::GetCKSettings();
	if (!Settings) return;

	const float Value = SliderToValue(NewValue, MinAimMultiplier, MaxAimMultiplier);
	Settings->SetAimSensitivityMultiplier(Value);

	if (AimSensitivityValueText)
		AimSensitivityValueText->SetText(FText::AsNumber(FMath::RoundToFloat(Value * 100.0f) / 100.0f));
}

void UCKSettingsWidget::OnCloseClicked()
{
	if (UCKGameUserSettings* Settings = UCKGameUserSettings::GetCKSettings())
		Settings->SaveCKSettings();

	RemoveFromParent();
}

float UCKSettingsWidget::ValueToSlider(float Value, float Min, float Max) const
{
	return FMath::Clamp((Value - Min) / FMath::Max(Max - Min, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
}

float UCKSettingsWidget::SliderToValue(float SliderValue, float Min, float Max) const
{
	return Min + (Max - Min) * FMath::Clamp(SliderValue, 0.0f, 1.0f);
}
