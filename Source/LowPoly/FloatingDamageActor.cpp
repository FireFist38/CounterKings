#include "FloatingDamageActor.h"
#include "FloatingDamageWidget.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"

AFloatingDamageActor::AFloatingDamageActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	DamageWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidget"));
	DamageWidgetComponent->SetupAttachment(SceneRoot);
	DamageWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	DamageWidgetComponent->SetDrawAtDesiredSize(true);
	DamageWidgetComponent->SetTwoSided(true);

	DamageWidgetClass = UFloatingDamageWidget::StaticClass();
}

void AFloatingDamageActor::InitializeDamage(float DamageAmount, EDamageType DamageType)
{
	CachedDamageAmount = DamageAmount;
	CachedDamageType = DamageType;
	RefreshWidget();
}

void AFloatingDamageActor::BeginPlay()
{
	Super::BeginPlay();

	const FVector2D RandomDirection(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f));
	DriftDirection = RandomDirection.GetSafeNormal();

	if (DamageWidgetClass)
	{
		DamageWidgetComponent->SetWidgetClass(DamageWidgetClass);
		DamageWidgetComponent->InitWidget();
	}

	RefreshWidget();
}

void AFloatingDamageActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedTime += DeltaSeconds;
	const FVector DriftStep(DriftDirection.X * LateralDriftSpeed * DeltaSeconds, DriftDirection.Y * LateralDriftSpeed * DeltaSeconds, RiseSpeed * DeltaSeconds);
	SetActorLocation(GetActorLocation() + DriftStep);

	const float LifetimeSafe = FMath::Max(Lifetime, KINDA_SMALL_NUMBER);
	const float NormalizedLife = FMath::Clamp(ElapsedTime / LifetimeSafe, 0.0f, 1.0f);
	const float FadeStart = FMath::Clamp(FadeStartNormalized, 0.0f, 0.99f);
	const float FadeAlpha = FMath::GetMappedRangeValueClamped(FVector2D(FadeStart, 1.0f), FVector2D(1.0f, 0.0f), NormalizedLife);

	if (UUserWidget* UserWidget = DamageWidgetComponent ? DamageWidgetComponent->GetUserWidgetObject() : nullptr)
	{
		UserWidget->SetRenderOpacity(FadeAlpha);

		const float PopAlpha = FMath::Clamp(ElapsedTime / FMath::Max(PopDuration, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
		const float CurrentScale = FMath::Lerp(InitialPopScale, 1.0f, PopAlpha);
		UserWidget->SetRenderScale(FVector2D(CurrentScale, CurrentScale));
	}

	if (NormalizedLife >= 1.0f)
	{
		Destroy();
	}
}

void AFloatingDamageActor::RefreshWidget()
{
	if (!DamageWidgetComponent)
	{
		return;
	}

	UFloatingDamageWidget* FloatingWidget = Cast<UFloatingDamageWidget>(DamageWidgetComponent->GetUserWidgetObject());
	if (FloatingWidget)
	{
		FloatingWidget->SetDamageData(CachedDamageAmount, CachedDamageType);
	}
}
