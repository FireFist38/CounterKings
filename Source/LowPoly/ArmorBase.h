#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "ArmorBase.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class LOWPOLY_API AArmorBase : public AItemBase
{
	GENERATED_BODY()

public:
	AArmorBase();

protected:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Stats")
	float HealthBonus;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Stats")
	float MovementSpeedModifier; // GDD 5.2: trade-offs

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
