#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.h"
#include "TestDummy.generated.h"

class UCapsuleComponent;
class UAttributeComponent;
class UWidgetComponent;

UCLASS()
class LOWPOLY_API ATestDummy : public AActor
{
	GENERATED_BODY()

public:
	ATestDummy();

	UPROPERTY(VisibleAnywhere, Category = "CK|Test") UCapsuleComponent* Capsule;
	UPROPERTY(VisibleAnywhere, Category = "CK|Test") UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere, Category = "CK|Test") UAttributeComponent* AttributeComponent;
	UPROPERTY(VisibleAnywhere, Category = "CK|Test") UWidgetComponent* HealthBarWidget;

	UPROPERTY(EditAnywhere, Category = "CK|Test")
	TSubclassOf<class UTestDummyHealthBar> HealthBarWidgetClass;

	// How often the dummy swings (seconds)
	UPROPERTY(EditAnywhere, Category = "CK|Test|Attack")
	float AttackInterval = 2.0f;

	// Radius of the sweep
	UPROPERTY(EditAnywhere, Category = "CK|Test|Attack")
	float AttackRange = 150.0f;

	// Damage values — tweak to test reduction
	UPROPERTY(EditAnywhere, Category = "CK|Test|AttackFields")
	FDamageBundle DamageBundle;

	UFUNCTION(BlueprintCallable, Category = "CK|Test")
	UAttributeComponent* GetAttributeComponent() const { return AttributeComponent; }

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle AttackTimerHandle;

	UFUNCTION()
	void PerformAttackSweep();
};
