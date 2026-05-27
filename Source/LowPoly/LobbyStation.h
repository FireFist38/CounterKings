#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CKInteractable.h"
#include "LobbyStation.generated.h"

UENUM(BlueprintType)
enum class ELobbyStationType : uint8
{
    Shop        UMETA(DisplayName = "Shop"),
    LevelUp     UMETA(DisplayName = "Level Up"),
    Equipment   UMETA(DisplayName = "Equipment"),
    ReadyUp     UMETA(DisplayName = "Ready Up")
};

UCLASS()
class LOWPOLY_API ALobbyStation : public AActor, public ICKInteractable
{
	GENERATED_BODY()
	
public:	
	ALobbyStation();

protected:
	virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Components")
    class USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Components")
    class UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Components")
    class USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Components")
    class UWidgetComponent* PromptWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Station")
    ELobbyStationType StationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Station")
    FText StationName;

public:
    // --- ICKInteractable Interface ---
    virtual void Interact_Implementation(APlayerCharacter* Interactor) override;
    virtual FText GetInteractionText_Implementation() const override;
};
