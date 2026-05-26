#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKXPBarWidget.generated.h"

class UHorizontalBox;
class UProgressBar;

UCLASS()
class LOWPOLY_API UCKXPBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* SegmentContainer;

	void UpdateXPBar(int32 Level, float CurrentXP, float XPPerLevel);

protected:
	virtual void NativeConstruct() override;
};
