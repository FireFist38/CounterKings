#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CKHUD.generated.h"

class UUserWidget;

UCLASS()
class LOWPOLY_API ACKHUD : public AHUD
{
	GENERATED_BODY()

public:
	ACKHUD();

	// The widget class to create (selected in Editor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

    UFUNCTION(BlueprintCallable, Category = "CK|UI")
    UUserWidget* GetHUDWidget() const { return CurrentHUD; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	UUserWidget* CurrentHUD;
};
