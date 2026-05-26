#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateColor.h"
#include "ItemBase.h"
#include "FloatingDamageWidget.generated.h"

class UTextBlock;

UCLASS()
class LOWPOLY_API UFloatingDamageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFloatingDamageWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "CK|UI")
	void SetDamageData(float DamageAmount, EDamageType DamageType);

	UFUNCTION(BlueprintCallable, Category = "CK|UI")
	void SetDamageValue(float DamageAmount);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DamageText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CK|UI|Colors")
	FLinearColor PhysicalColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CK|UI|Colors")
	FLinearColor MagicColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CK|UI|Colors")
	FLinearColor FireColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CK|UI|Colors")
	FLinearColor LightningColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CK|UI|Colors")
	FLinearColor FrostColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CK|UI|Colors")
	FLinearColor PoisonColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CK|UI|Colors")
	FLinearColor HolyColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CK|UI|Colors")
	FLinearColor EarthColor;

private:
	FSlateColor GetDamageTypeColor(EDamageType DamageType) const;
};
