#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class USphereComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LOWPOLY_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractionComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Interaction")
	USphereComponent* InteractionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Interaction")
	float InteractionRadius;

public:	
	// Called by Character via Enhanced Input
	UFUNCTION(BlueprintCallable, Category = "CK|Interaction")
	void PerformInteraction();

    /** Returns the best interactable object currently in range */
    UFUNCTION(BlueprintCallable, Category = "CK|Interaction")
    AActor* GetBestInteractable();

private:
	UFUNCTION(Server, Reliable)
	void Server_PerformInteraction();
};
