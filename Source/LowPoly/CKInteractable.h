#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CKInteractable.generated.h"

class APlayerCharacter;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCKInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for any actor that can be interacted with (Items, Lobby Stations, etc.)
 */
class LOWPOLY_API ICKInteractable
{
	GENERATED_BODY()

public:
    /** 
     * Called on the server when a player interacts with this object.
     * @param Interactor The character performing the interaction.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CK|Interaction")
    void Interact(APlayerCharacter* Interactor);

    /**
     * Returns the text to display on the HUD when looking at this object.
     * @return FText for the interaction prompt.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CK|Interaction")
    FText GetInteractionText() const;
};
