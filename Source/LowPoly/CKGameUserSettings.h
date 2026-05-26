#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "CKGameUserSettings.generated.h"

UCLASS(BlueprintType, Config = GameUserSettings)
class LOWPOLY_API UCKGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UCKGameUserSettings(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "CK|Settings")
	static UCKGameUserSettings* GetCKSettings();

	UFUNCTION(BlueprintCallable, Category = "CK|Settings")
	float GetMouseSensitivity() const { return MouseSensitivity; }

	UFUNCTION(BlueprintCallable, Category = "CK|Settings")
	void SetMouseSensitivity(float NewValue);

	UFUNCTION(BlueprintCallable, Category = "CK|Settings")
	float GetAimSensitivityMultiplier() const { return AimSensitivityMultiplier; }

	UFUNCTION(BlueprintCallable, Category = "CK|Settings")
	void SetAimSensitivityMultiplier(float NewValue);

	UFUNCTION(BlueprintCallable, Category = "CK|Settings")
	void SaveCKSettings();

protected:
	UPROPERTY(Config)
	float MouseSensitivity = 1.0f;

	UPROPERTY(Config)
	float AimSensitivityMultiplier = 0.5f;
};
