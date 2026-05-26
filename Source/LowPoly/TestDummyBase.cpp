#include "TestDummyBase.h"
#include "AttributeComponent.h"
#include "GameFramework/Actor.h"

ATestDummyBase::ATestDummyBase()
{
}

void ATestDummyBase::TriggerTestDamage(AActor* TargetActor)
{
    if (!TargetActor) return;

    // Find the attribute component on the target
    UAttributeComponent* TargetAttributes = TargetActor->FindComponentByClass<UAttributeComponent>();
    if (TargetAttributes)
    {
        TargetAttributes->ApplyCombatDamage(TestDamage, this);
    }
}
