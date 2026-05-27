#include "SpellIcicleDrop.h"
#include "ProjectileBase.h"
#include "PlayerCharacter.h"
#include "MagicWeaponBase.h"

USpellIcicleDrop::USpellIcicleDrop()
{
	DisplayName = FName("Icicle Drop");
	ManaCost = 30.0f;
	Cooldown = 1.0f;
    TargetRadius = 300.0f;
}

void USpellIcicleDrop::OnDetonate(APlayerCharacter* Caster, AMagicWeaponBase* Staff, const FVector& TargetLocation)
{
	if (!Caster || !Caster->HasAuthority()) return;

	if (IcicleClass)
	{
		// Calculate spawn point high above the target
		FVector SpawnLocation = TargetLocation + FVector(0.f, 0.f, DropHeight);
		
		// Rotation pointing straight down
		FRotator SpawnRotation = FRotator(-90.f, 0.f, 0.f);

		FActorSpawnParameters Params;
		Params.Owner = Caster;
		Params.Instigator = Caster;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AProjectileBase* Icicle = GetWorld()->SpawnActor<AProjectileBase>(IcicleClass, SpawnLocation, SpawnRotation, Params);
		if (Icicle)
		{
            // Configure the projectile for vertical drop
            Icicle->InitialSpeed = InitialDropSpeed;
            Icicle->ExplosionRadius = TargetRadius; // Match the indicator radius
            
            // Pass damage and shooter info
			Icicle->InitProjectile(Caster, DetonationDamage);
		}
	}
}
