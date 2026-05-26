#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ItemBase.h"
#include "TestDummyBase.generated.h"

/**
 * C++ Base class for Test Dummies to handle damage type testing.
 */
UCLASS()
class LOWPOLY_API ATestDummyBase : public ACharacter
{
	GENERATED_BODY()

public:
	ATestDummyBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Test|AttackFields")
	FDamageBundle TestDamage;

	UFUNCTION(BlueprintCallable, Category = "CK|Test")
	void TriggerTestDamage(AActor* TargetActor);
};
