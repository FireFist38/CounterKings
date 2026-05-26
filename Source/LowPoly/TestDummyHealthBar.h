#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TestDummyHealthBar.generated.h"

class UProgressBar;
class ATestDummy;

UCLASS()
class LOWPOLY_API UTestDummyHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget)) UProgressBar* HealthBar;

	UPROPERTY(BlueprintReadWrite, Category = "CK|Test")
	ATestDummy* OwningDummy;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
