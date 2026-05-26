#include "CKHUD.h"
#include "Blueprint/UserWidget.h"

ACKHUD::ACKHUD()
{
}

void ACKHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		CurrentHUD = CreateWidget<UUserWidget>(GetOwningPlayerController(), HUDWidgetClass);
		if (CurrentHUD)
		{
			CurrentHUD->AddToViewport();
		}
	}
}
