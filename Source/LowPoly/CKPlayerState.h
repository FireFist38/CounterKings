#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ItemBase.h"
#include "CKPlayerState.generated.h"

UCLASS()
class LOWPOLY_API ACKPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ACKPlayerState();

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Match")
	int32 MatchHealth = 100;

    // --- Persistent Data (survives ServerTravel) ---

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedLevel = 1;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    float SavedXP = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedGold = 0;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    int32 SavedAttributePoints = 0;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    float SavedStrength = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    float SavedDexterity = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    float SavedMagic = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    float SavedLuck = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<FName> SavedShopPool;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Persist")
    TArray<bool> SavedLockedSlots;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};