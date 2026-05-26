#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CKPlayerState.generated.h"

UCLASS()
class LOWPOLY_API ACKPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ACKPlayerState();

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Match")
	int32 MatchHealth = 100;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
