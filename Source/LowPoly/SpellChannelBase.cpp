#include "SpellChannelBase.h"
#include "MagicWeaponBase.h"
#include "AttributeComponent.h"
#include "PlayerCharacter.h"

bool USpellChannelBase::StartChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff)
{
    if (Caster)
    {
        // Use the configurable angle from the class defaults
        Caster->SetRotationClamp(true, ChannelRotationClampAngle);
    }
    return true;
}

bool USpellChannelBase::TickChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff, float DeltaTime)
{
    if (!Caster || !Caster->GetAttributeComponent()) return false;

    UAttributeComponent* Attr = Caster->GetAttributeComponent();
    float TotalDrain = DrainPerSecond * DeltaTime;

    return bUseMana ? Attr->ConsumeMana(TotalDrain) : Attr->ConsumeStamina(TotalDrain);
}

void USpellChannelBase::EndChannel(APlayerCharacter* Caster, AMagicWeaponBase* Staff)
{
    APlayerCharacter* FinalCaster = Caster;

    // Fallback: If Caster was passed as null, try to find it via the staff's owner.
    if (!FinalCaster && Staff)
    {
        AActor* StaffOwner = Staff->GetOwner();
        if (StaffOwner)
        {
            // Use ::Cast to avoid collision with USpellBase::Cast() member function
            FinalCaster = ::Cast<APlayerCharacter>(StaffOwner);
        }
    }

    if (FinalCaster)
    {
        FinalCaster->SetRotationClamp(false, 0.0f);
    }
}
