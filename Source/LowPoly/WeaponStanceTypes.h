#pragma once

#include "CoreMinimal.h"
#include "WeaponStanceTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponHoldStance : uint8
{
	Empty            UMETA(DisplayName = "Empty"),
	OneHandedMelee   UMETA(DisplayName = "One-Handed Melee"),
	TwoHandedMelee   UMETA(DisplayName = "Two-Handed Melee"),
	OneHandedRanged  UMETA(DisplayName = "One-Handed Ranged"),
	TwoHandedRanged  UMETA(DisplayName = "Two-Handed Ranged"),
	Tome             UMETA(DisplayName = "Tome"),
	Throwable        UMETA(DisplayName = "Throwable"),
	OffHandPistol    UMETA(DisplayName = "Off-Hand Pistol")
};
