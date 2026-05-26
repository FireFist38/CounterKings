#include "CKGameUserSettings.h"

UCKGameUserSettings::UCKGameUserSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UCKGameUserSettings* UCKGameUserSettings::GetCKSettings()
{
	return Cast<UCKGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UCKGameUserSettings::SetMouseSensitivity(float NewValue)
{
	MouseSensitivity = FMath::Clamp(NewValue, 0.01f, 10.0f);
}

void UCKGameUserSettings::SetAimSensitivityMultiplier(float NewValue)
{
	AimSensitivityMultiplier = FMath::Clamp(NewValue, 0.01f, 5.0f);
}

void UCKGameUserSettings::SaveCKSettings()
{
	SaveSettings();
}
