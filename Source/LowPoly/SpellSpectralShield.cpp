#include "SpellSpectralShield.h"
#include "SpectralShieldBase.h"
#include "PlayerCharacter.h"
#include "MagicWeaponBase.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"

USpellSpectralShield::USpellSpectralShield()
{
	DisplayName = FName("Spectral Shield");
	DrainPerSecond = 15.0f;
	bUseMana = true;
	Cooldown = 2.0f;
}

bool USpellSpectralShield::StartChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff)
{
	if (!Caster || !ShieldClass) 
    {
        return false;
    }

	UWorld* World = Caster->GetWorld();
	if (!World) return false;

	// Destroy any previous shield first
	if (Caster->ActiveSpectralShield)
	{
		Caster->ActiveSpectralShield->Destroy();
		Caster->ActiveSpectralShield = nullptr;
	}

	FActorSpawnParameters Params;
	Params.Owner = Caster;
	Params.Instigator = Caster;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn at the caster's location first; TickComponent will reposition it
	ASpectralShieldBase* Shield = World->SpawnActor<ASpectralShieldBase>(ShieldClass, Caster->GetActorLocation(), FRotator::ZeroRotator, Params);
	
	if (Shield)
	{
		if (!ShieldExtent.IsNearlyZero())
		{
			Shield->ConfigureShield(ShieldExtent);
		}

		Shield->SetCaster(Caster);
		Caster->ActiveSpectralShield = Shield;

		Caster->SetIsBlocking(true);
		Caster->CustomMovementMultiplier = MovementSlowMultiplier;
		Caster->CustomRotationMultiplier = RotationSlowMultiplier;
	}

	return Shield != nullptr;
}

bool USpellSpectralShield::TickChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff, float DeltaTime)
{
    if (!Caster)
    {
        return false;
    }

    // Update shield position every tick to track caster's YAW rotation
    if (Caster->ActiveSpectralShield)
    {
        // Position the shield in front of the caster at camera height
        // Only use YAW rotation so the shield doesn't tilt up/down with camera pitch
        AController* CasterController = Caster->GetController();
        FRotator YawRotation = Caster->GetActorRotation();
        if (CasterController)
        {
            YawRotation = CasterController->GetControlRotation();
        }
        YawRotation.Pitch = 0.0f;
        YawRotation.Roll = 0.0f;

        const FVector Forward = YawRotation.Vector();
        const FVector ShieldPos = Caster->GetActorLocation() + (Forward * ShieldOffset.X) + FVector(0.0f, 0.0f, ShieldOffset.Z);

        Caster->ActiveSpectralShield->SetActorLocation(ShieldPos);
        Caster->ActiveSpectralShield->SetActorRotation(YawRotation + ShieldRotation);
    }

    // Super handles mana drain
    return Super::TickChannel(Caster, Staff, DeltaTime);
}

void USpellSpectralShield::EndChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff)
{
	Super::EndChannel(Caster, Staff);

	if (Caster && Caster->ActiveSpectralShield)
	{
		Caster->ActiveSpectralShield->Destroy();
		Caster->ActiveSpectralShield = nullptr;
	}

	if (Caster)
	{
		Caster->SetIsBlocking(false);
		Caster->CustomMovementMultiplier = 1.0f;
		Caster->CustomRotationMultiplier = 1.0f;
	}
}