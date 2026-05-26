#include "SpellTargetedAOE.h"
#include "TargetedAOEIndicator.h"
#include "TargetedAOEDetonation.h"
#include "PlayerCharacter.h"
#include "MagicWeaponBase.h"

USpellTargetedAOE::USpellTargetedAOE()
{
    DisplayName = FName("Targeted AOE");
    ManaCost = 25.0f;
    Cooldown = 0.5f;
}

bool USpellTargetedAOE::Cast(APlayerCharacter* Caster, AMagicWeaponBase* Staff)
{
    // SpellBase::Cast returns true by default if not overridden with failure logic.
    // We don't call Super::Cast because it's abstract/empty.

    if (!Caster || !Staff) return false;

    if (!bIsTargeting)
    {
        // Phase 1: Spawn Indicator
        if (IndicatorClass)
        {
            FActorSpawnParameters Params;
            Params.Owner = Caster;
            Params.Instigator = Caster;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            ActiveIndicator = GetWorld()->SpawnActor<ATargetedAOEIndicator>(IndicatorClass, Caster->GetActorLocation(), FRotator::ZeroRotator, Params);
            if (ActiveIndicator)
            {
                ActiveIndicator->InitIndicator(TargetRadius, MaxTargetRange);
                bIsTargeting = true;
                return true;
            }
        }
    }
    else
    {
        // Phase 2: Detonate
        if (ActiveIndicator)
        {
            FVector DetonationLocation = ActiveIndicator->GetActorLocation();
            
            // Spawn Detonation actor
            if (DetonationClass)
            {
                FActorSpawnParameters Params;
                Params.Owner = Caster;
                Params.Instigator = Caster;
                Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                ATargetedAOEDetonation* Detonation = GetWorld()->SpawnActor<ATargetedAOEDetonation>(DetonationClass, DetonationLocation, FRotator::ZeroRotator, Params);
                if (Detonation)
                {
                    Detonation->DamageRadius = TargetRadius;
                    Detonation->DetonationLifetime = DetonationLifetime;
                    Detonation->InitDetonation(Caster, DetonationDamage);
                }
            }

            // Cleanup indicator
            ActiveIndicator->Destroy();
            ActiveIndicator = nullptr;
            bIsTargeting = false;
            return true;
        }
        else
        {
            // Indicator was lost or destroyed somehow, reset state
            bIsTargeting = false;
            // Recursively call to start targeting again
            return Cast(Caster, Staff);
        }
    }

    return false;
}
