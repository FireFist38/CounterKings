#include "SpellProjectile.h"
#include "MagicWeaponBase.h"
#include "PlayerCharacter.h"
#include "ProjectileBase.h"
#include "AttributeComponent.h"

USpellProjectile::USpellProjectile()
{
	DisplayName = FName("Magic Bolt");
	ManaCost = 15.0f;
	Cooldown = 0.4f;
    SpellDamage.Magic = 25.0f;
}

bool USpellProjectile::Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff)
{
	if (!Caster || !Staff || !ProjectileClass) return false;
	UWorld* World = Caster->GetWorld();
	if (!World) return false;

	const FVector Origin = GetCastOrigin(Staff);
	const FRotator Rotation = GetCastRotation(Caster);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Caster;
	SpawnParams.Instigator = Caster;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AProjectileBase* Projectile = World->SpawnActor<AProjectileBase>(ProjectileClass, Origin, Rotation, SpawnParams);
	if (!Projectile) return false;

	// Scale damage through the staff so attribute scalings (Magic/Dex/Str) apply
	UAttributeComponent* Attr = Caster->GetAttributeComponent();
	const float Mult = Staff->ComputeAttributeMultiplier(Attr);
    
    // Total Damage = (Spell's own internal damage + the Staff's Rarity-based damage) * Attributes
    FDamageBundle FinalDamage = SpellDamage;
    FDamageBundle StaffBonus = Staff->GetCurrentDamage();
    
    FinalDamage.Physical += StaffBonus.Physical;
    FinalDamage.Magic += StaffBonus.Magic;
    FinalDamage.Fire += StaffBonus.Fire;
    FinalDamage.Lightning += StaffBonus.Lightning;
    FinalDamage.Frost += StaffBonus.Frost;
    FinalDamage.Poison += StaffBonus.Poison;
    FinalDamage.Holy += StaffBonus.Holy;
    FinalDamage.Earth += StaffBonus.Earth;

    FinalDamage.Scale(Mult);

	Projectile->InitProjectile(Caster, FinalDamage);

	return true;
}
