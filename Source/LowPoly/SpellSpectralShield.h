#pragma once

#include "CoreMinimal.h"
#include "SpellChannelBase.h"
#include "SpectralShieldBase.h"
#include "SpellSpectralShield.generated.h"

/**
 * Channeled spell that spawns a physical shield actor.
 * Follows the player's camera view and slows movement/rotation.
 */
UCLASS(Blueprintable, EditInlineNew)
class LOWPOLY_API USpellSpectralShield : public USpellChannelBase
{
	GENERATED_BODY()

public:
	USpellSpectralShield();

	virtual bool StartChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff) override;
	virtual bool TickChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff, float DeltaTime) override;
	virtual void EndChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff) override;

	/** The class of the shield actor to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Shield")
	TSubclassOf<ASpectralShieldBase> ShieldClass;

	/** 
	 * Position of the shield relative to the camera view.
	 * X is Forward, Y is Right, Z is Up.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Shield")
	FVector ShieldOffset = FVector(120.0f, 0.0f, 0.0f);

	/** Rotation offset for the shield relative to the camera view. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Shield")
	FRotator ShieldRotation = FRotator::ZeroRotator;

	/** 
	 * Forced collision extent. 
	 * Leave at (0, 0, 0) to use the values set in the Shield Actor Blueprint.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Shield")
	FVector ShieldExtent = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Shield")
	float MovementSlowMultiplier = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Spell|Shield")
	float RotationSlowMultiplier = 0.3f;

protected:
	UPROPERTY()
	ASpectralShieldBase* SpawnedShield;
};
