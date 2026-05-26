#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "ConsumableBase.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class LOWPOLY_API AConsumableBase : public AItemBase
{
	GENERATED_BODY()

public:
	AConsumableBase();

protected:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CK|Consumable")
	int32 UsesRemaining;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
