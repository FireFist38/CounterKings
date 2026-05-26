#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKScopeOverlayWidget.generated.h"

class UImage;

// Base class for scope overlay widgets. Create BP children with optional bound widgets:
//   - ScopeRing: image of the black border surrounding the scope lens
//   - ScopeReticle: image of the crosshair/reticle inside the lens
// Both are optional — design the visual entirely in BP if you want.
UCLASS(Abstract)
class LOWPOLY_API UCKScopeOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* ScopeRing;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* ScopeReticle;
};
