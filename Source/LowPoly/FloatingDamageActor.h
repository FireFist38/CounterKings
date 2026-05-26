#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.h"
#include "FloatingDamageActor.generated.h"

class UWidgetComponent;
class UFloatingDamageWidget;

UCLASS()
class LOWPOLY_API AFloatingDamageActor : public AActor
{
	GENERATED_BODY()

public:
	AFloatingDamageActor();

	UFUNCTION(BlueprintCallable, Category = "CK|UI")
	void InitializeDamage(float DamageAmount, EDamageType DamageType);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Components")
	UWidgetComponent* DamageWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|UI")
	TSubclassOf<UFloatingDamageWidget> DamageWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|UI")
	float Lifetime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|UI")
	float RiseSpeed = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|UI")
	float LateralDriftSpeed = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|UI")
	float InitialPopScale = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|UI")
	float PopDuration = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|UI")
	float FadeStartNormalized = 0.25f;

private:
	void RefreshWidget();

	float ElapsedTime = 0.0f;
	float CachedDamageAmount = 0.0f;
	EDamageType CachedDamageType = EDamageType::Physical;
	FVector2D DriftDirection = FVector2D::ZeroVector;
};
