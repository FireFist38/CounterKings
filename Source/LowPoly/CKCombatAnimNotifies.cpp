#include "CKCombatAnimNotifies.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlayerCharacter.h"

namespace
{
	APlayerCharacter* GetNotifyCharacter(const USkeletalMeshComponent* MeshComp)
	{
		return MeshComp ? Cast<APlayerCharacter>(MeshComp->GetOwner()) : nullptr;
	}
}

void UCKAnimNotify_OpenComboWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (APlayerCharacter* Character = GetNotifyCharacter(MeshComp))
	{
		Character->AnimNotify_OpenComboWindow();
	}
}

void UCKAnimNotify_CloseComboWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (APlayerCharacter* Character = GetNotifyCharacter(MeshComp))
	{
		Character->AnimNotify_CloseComboWindow();
	}
}

void UCKAnimNotify_CommitCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (APlayerCharacter* Character = GetNotifyCharacter(MeshComp))
	{
		Character->AnimNotify_CommitCombo();
	}
}

void UCKAnimNotify_Lunge::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (APlayerCharacter* Character = GetNotifyCharacter(MeshComp))
    {
        Character->AnimNotify_Lunge(LungeDistance, LungeSpeed);
    }
}

void UCKAnimNotify_ResetCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (APlayerCharacter* Character = GetNotifyCharacter(MeshComp))
	{
		Character->AnimNotify_ResetCombo();
	}
}
