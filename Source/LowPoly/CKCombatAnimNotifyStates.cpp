#include "CKCombatAnimNotifyStates.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlayerCharacter.h"

namespace
{
	APlayerCharacter* GetStateNotifyCharacter(const USkeletalMeshComponent* MeshComp)
	{
		return MeshComp ? Cast<APlayerCharacter>(MeshComp->GetOwner()) : nullptr;
	}
}

void UCKAnimNotifyState_MeleeHitWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (APlayerCharacter* Character = GetStateNotifyCharacter(MeshComp))
	{
		Character->AnimNotify_MeleeHitWindowBegin();
	}
}

void UCKAnimNotifyState_MeleeHitWindow::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (APlayerCharacter* Character = GetStateNotifyCharacter(MeshComp))
	{
		Character->AnimNotify_MeleeHitWindowTick(FrameDeltaTime);
	}
}

void UCKAnimNotifyState_MeleeHitWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (APlayerCharacter* Character = GetStateNotifyCharacter(MeshComp))
	{
		Character->AnimNotify_MeleeHitWindowEnd();
	}
}
