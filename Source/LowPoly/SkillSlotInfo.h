#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "SkillSlotInfo.generated.h"

// Data the HUD uses to render one skill slot (LMB or RMB).
USTRUCT(BlueprintType)
struct LOWPOLY_API FSkillSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UTexture2D* Icon = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;

	// 1.0 = ready, 0.0 = just used and on cooldown
	UPROPERTY(BlueprintReadOnly)
	float CooldownProgress = 1.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bHasCooldown = false;

	// If false, the slot is empty/hidden
	UPROPERTY(BlueprintReadOnly)
	bool bIsValid = false;
};
