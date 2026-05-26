#include "AbilityBase.h"
#include "PlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

AAbilityBase::AAbilityBase()
{
    ItemType = EItemType::Consumable; // Re-use an existing enum or add EItemType::Ability later
    bReplicates = true;
}

bool AAbilityBase::CanActivate(AActor* Character) const
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(Character);
    if (!Player) return false;

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastActivationTime) >= Cooldown;
}

void AAbilityBase::ActivateAbility(AActor* Character)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(Character);
    if (!HasAuthority() || !Player || !CanActivate(Player)) return;

    LastActivationTime = GetWorld()->GetTimeSeconds();

    if (ActivationMontage)
    {
        Player->Multicast_PlayAttackMontage(ActivationMontage, NAME_None);
    }

    if (ActivationSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ActivationSound, Player->GetActorLocation());
    }

    BP_OnActivated(Player);
}

float AAbilityBase::GetCooldownRemaining() const
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const float Elapsed = CurrentTime - LastActivationTime;
    return FMath::Max(0.0f, Cooldown - Elapsed);
}

float AAbilityBase::GetCooldownNormalized() const
{
    if (Cooldown <= 0.0f) return 1.0f;
    const float Remaining = GetCooldownRemaining();
    return FMath::Clamp(1.0f - (Remaining / Cooldown), 0.0f, 1.0f);
}

void AAbilityBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AAbilityBase, LastActivationTime);
}
