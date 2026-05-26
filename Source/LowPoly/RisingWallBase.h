#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RisingWallBase.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class LOWPOLY_API ARisingWallBase : public AActor
{
	GENERATED_BODY()

public:
	ARisingWallBase();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "CK|Wall")
	void ConfigureWall(float InLifetime, float InRiseDuration, float InRiseHeight);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Wall")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Wall")
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Wall")
	UStaticMeshComponent* WallMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Wall", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float Lifetime = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Wall", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float RiseDuration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Wall", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RiseHeight = 180.0f;

private:
	float RiseElapsed = 0.0f;
	bool bHasReachedFinalHeight = false;
	FVector TargetLocation = FVector::ZeroVector;
	FVector StartLocation = FVector::ZeroVector;
};
