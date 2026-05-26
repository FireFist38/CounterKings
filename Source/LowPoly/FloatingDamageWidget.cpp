#include "FloatingDamageWidget.h"
#include "Components/TextBlock.h"

UFloatingDamageWidget::UFloatingDamageWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PhysicalColor = FLinearColor(0.92f, 0.92f, 0.92f, 1.0f);
	MagicColor = FLinearColor(0.10f, 0.22f, 0.65f, 1.0f);
	FireColor = FLinearColor(1.0f, 0.34f, 0.08f, 1.0f);
	LightningColor = FLinearColor(1.0f, 0.88f, 0.15f, 1.0f);
	FrostColor = FLinearColor(0.35f, 0.87f, 1.0f, 1.0f);
	PoisonColor = FLinearColor(0.43f, 0.86f, 0.27f, 1.0f);
	HolyColor = FLinearColor(1.0f, 0.79f, 0.28f, 1.0f);
	EarthColor = FLinearColor(0.52f, 0.36f, 0.19f, 1.0f);
}

void UFloatingDamageWidget::SetDamageData(float DamageAmount, EDamageType DamageType)
{
	if (!DamageText)
	{
		return;
	}

	DamageText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), DamageAmount)));
	DamageText->SetColorAndOpacity(GetDamageTypeColor(DamageType));
	DamageText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));
	DamageText->SetShadowOffset(FVector2D(1.5f, 1.5f));
}

void UFloatingDamageWidget::SetDamageValue(float DamageAmount)
{
	SetDamageData(DamageAmount, EDamageType::Physical);
}

FSlateColor UFloatingDamageWidget::GetDamageTypeColor(EDamageType DamageType) const
{
	switch (DamageType)
	{
	case EDamageType::Physical:
		return FSlateColor(PhysicalColor);
	case EDamageType::Magic:
		return FSlateColor(MagicColor);
	case EDamageType::Fire:
		return FSlateColor(FireColor);
	case EDamageType::Lightning:
		return FSlateColor(LightningColor);
	case EDamageType::Frost:
		return FSlateColor(FrostColor);
	case EDamageType::Poison:
		return FSlateColor(PoisonColor);
	case EDamageType::Holy:
		return FSlateColor(HolyColor);
	case EDamageType::Earth:
		return FSlateColor(EarthColor);
	default:
		return FSlateColor(PhysicalColor);
	}
}
