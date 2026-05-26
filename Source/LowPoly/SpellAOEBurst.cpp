#include "SpellAOEBurst.h"
#include "AOEBurstBase.h"
#include "PlayerCharacter.h"
#include "MagicWeaponBase.h"
#include "AttributeComponent.h"
#include "Engine/World.h"

USpellAOEBurst::USpellAOEBurst()
{
	DisplayName = FName("AOE Burst");
	ManaCost = 35.0f;
	StaminaCost = 0.0f;
	bUseMana = true;
	Cooldown = 6.0f;
	CastTime = 0.0f;

	BurstDamage.Physical = 10.0f;
	BurstDamage.Magic = 20.0f;
}

bool USpellAOEBurst::Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff)
{
	if (!Caster || !BurstClass) return false;

	UWorld* World = Caster->GetWorld();
	if (!World) return false;

	// Scale damage through the caster's attributes
	UAttributeComponent* Attr = Caster->GetAttributeComponent();
	FDamageBundle ScaledDamage = BurstDamage;
	if (Staff && Attr)
	{
		ScaledDamage.Scale(Staff->ComputeAttributeMultiplier(Attr));
	}

	// Spawn the burst centered on the caster
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Caster;
	SpawnParams.Instigator = Caster;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAOEBurstBase* Burst = World->SpawnActor<AAOEBurstBase>(
		BurstClass,
		Caster->GetActorLocation(),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!Burst) return false;

	Burst->MaxRadius = MaxRadius;
	Burst->ExpansionSpeed = ExpansionSpeed;
	Burst->DamageHalfHeight = DamageHalfHeight;

	Burst->InitBurst(Caster, ScaledDamage);

	return true;
}
