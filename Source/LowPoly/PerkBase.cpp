#include "PerkBase.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"

APerkBase::APerkBase()
{
	ItemType = EItemType::Perk;
	bReplicates = true;
}

void APerkBase::OnPerkActivated(AActor* Character)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(Character);
    if (!Player) return;
    OwningActor = Player;

    // Apply attribute bonuses on the server (replicates to clients via AttributeComponent).
    if (UAttributeComponent* Attr = Player->GetAttributeComponent())
    {
        Attr->ApplyStrengthBonus(StrengthBonus);
        Attr->ApplyDexterityBonus(DexterityBonus);
        Attr->ApplyMagicBonus(MagicBonus);
        Attr->ApplyLuckBonus(LuckBonus);

        Attr->ApplyHealthBonus(HealthBonus);
    }

    BP_OnPerkActivated(Player);
}

void APerkBase::OnPerkDeactivated(AActor* Character)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(Character);
    if (Player)
    {
        if (UAttributeComponent* Attr = Player->GetAttributeComponent())
        {
            Attr->RemoveStrengthBonus(StrengthBonus);
            Attr->RemoveDexterityBonus(DexterityBonus);
            Attr->RemoveMagicBonus(MagicBonus);
            Attr->RemoveLuckBonus(LuckBonus);

            Attr->RemoveHealthBonus(HealthBonus);
        }
    }

    BP_OnPerkDeactivated(Player);
    OwningActor = nullptr;
}

void APerkBase::ModifyIncomingDamage(FDamageBundle& InOutDamage, AActor* DamageInstigator)
{
    // Apply damage type negation percentages (0.0 to 1.0)
    // 0.0 = 0% negation, 1.0 = 100% negation
    InOutDamage.Physical *= (1.0f - PhysicalNegation);
    InOutDamage.Magic *= (1.0f - MagicNegation);
    InOutDamage.Fire *= (1.0f - FireNegation);
    InOutDamage.Lightning *= (1.0f - LightningNegation);
    InOutDamage.Frost *= (1.0f - FrostNegation);
    InOutDamage.Poison *= (1.0f - PoisonNegation);
    InOutDamage.Holy *= (1.0f - HolyNegation);
    InOutDamage.Earth *= (1.0f - EarthNegation);

    // Call Blueprint event for custom logic
    BP_ModifyIncomingDamage(InOutDamage, DamageInstigator);
}

void APerkBase::ModifyOutgoingDamage(FDamageBundle& InOutDamage, AActor* Target)
{
    // Apply damage type multipliers
    InOutDamage.Physical *= PhysicalDamageMultiplier;
    InOutDamage.Magic *= MagicDamageMultiplier;
    InOutDamage.Fire *= FireDamageMultiplier;
    InOutDamage.Lightning *= LightningDamageMultiplier;
    InOutDamage.Frost *= FrostDamageMultiplier;
    InOutDamage.Poison *= PoisonDamageMultiplier;
    InOutDamage.Holy *= HolyDamageMultiplier;
    InOutDamage.Earth *= EarthDamageMultiplier;

    // Call Blueprint event for custom logic
    BP_ModifyOutgoingDamage(InOutDamage, Target);
}
