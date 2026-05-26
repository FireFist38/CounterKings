#include "SpellBase.h"
#include "MagicWeaponBase.h"
#include "PlayerCharacter.h"

bool USpellBase::Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff)
{
	// Base implementation does nothing — subclasses override
	return false;
}

FVector USpellBase::GetCastOrigin(AMagicWeaponBase* Staff) const
{
	if (!Staff) return FVector::ZeroVector;
    
    // Iterate through all mesh components to find the one with the socket
    TArray<UStaticMeshComponent*> MeshComponents;
    Staff->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    for (UStaticMeshComponent* SM : MeshComponents)
    {
        if (SM && SM->DoesSocketExist(Staff->GetCastSocketName()))
        {
            return SM->GetSocketLocation(Staff->GetCastSocketName());
        }
    }
    
	return Staff->GetActorLocation();
}

FRotator USpellBase::GetCastRotation(APlayerCharacter* Caster) const
{
	if (!Caster) return FRotator::ZeroRotator;
	if (AController* C = Caster->GetController())
		return C->GetControlRotation();
	return Caster->GetActorRotation();
}
