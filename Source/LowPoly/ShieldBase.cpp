#include "ShieldBase.h"
#include "PlayerCharacter.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

AShieldBase::AShieldBase()
{
    BlockMontage = nullptr;
    BlockSound = nullptr;
    BlockEffect = nullptr;
    BlockEffectScale = 1.0f;
    PhysicalNegation = 0.8f;
    MagicNegation = 0.2f;
    StaminaCostOnBlock = 10.0f;
}

void AShieldBase::UseOffHand(class APlayerCharacter* Character)
{
    if (Character)
    {
        Character->SetIsBlocking(true);
        if (BlockMontage)
        {
            Character->Multicast_PlayAttackMontage(BlockMontage, NAME_None);
        }
    }
}

void AShieldBase::Multicast_OnBlocked_Implementation()
{
    if (BlockSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BlockSound, GetActorLocation());
    }

    if (BlockEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            BlockEffect,
            GetActorLocation(),
            GetActorRotation(),
            FVector(BlockEffectScale),
            true,
            true,
            ENCPoolMethod::None,
            true
        );
    }
}

void AShieldBase::StopUseOffHand(class APlayerCharacter* Character)
{
    if (Character)
    {
        Character->SetIsBlocking(false);
        if (Character->GetMesh())
        {
            UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
            if (AnimInstance && AnimInstance->Montage_IsPlaying(BlockMontage))
            {
                AnimInstance->Montage_Stop(0.2f, BlockMontage);
            }
        }
    }
}