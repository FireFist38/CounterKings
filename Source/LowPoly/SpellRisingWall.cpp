#include "SpellRisingWall.h"
#include "RisingWallBase.h"
#include "PlayerCharacter.h"

USpellRisingWall::USpellRisingWall()
{
	DisplayName = FName("Rising Wall");
	ManaCost = 25.0f;
	StaminaCost = 0.0f;
	bUseMana = true;
	Cooldown = 10.0f;
	CastTime = 0.0f;
	WallClass = ARisingWallBase::StaticClass();
}

bool USpellRisingWall::Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff)
{
	if (!Caster || !WallClass)
	{
		return false;
	}

	UWorld* World = Caster->GetWorld();
	if (!World)
	{
		return false;
	}

	const FRotator AimRotation = GetCastRotation(Caster);
	const FRotator YawRotation(0.0f, AimRotation.Yaw, 0.0f);
	const FVector Forward = YawRotation.Vector();

	const FVector Origin = Caster->GetActorLocation() + Forward * SpawnDistance;
	const FVector TraceStart = Origin + FVector(0.0f, 0.0f, GroundTraceHalfHeight);
	const FVector TraceEnd = Origin - FVector(0.0f, 0.0f, GroundTraceHalfHeight);

	FHitResult GroundHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Caster);

	const bool bHit = World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, Params);
	const FVector SpawnLocation = bHit ? GroundHit.ImpactPoint : Origin;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Caster;
	SpawnParams.Instigator = Caster;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ARisingWallBase* Wall = World->SpawnActor<ARisingWallBase>(WallClass, SpawnLocation, YawRotation, SpawnParams);
	if (!Wall)
	{
		return false;
	}

	Wall->ConfigureWall(WallLifetime, WallRiseDuration, WallRiseHeight);
	return true;
}
