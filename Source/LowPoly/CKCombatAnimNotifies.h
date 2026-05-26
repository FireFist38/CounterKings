#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CKCombatAnimNotifies.generated.h"

UCLASS(DisplayName = "CK Open Combo Window")
class LOWPOLY_API UCKAnimNotify_OpenComboWindow : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};

UCLASS(DisplayName = "CK Close Combo Window")
class LOWPOLY_API UCKAnimNotify_CloseComboWindow : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};

UCLASS(DisplayName = "CK Commit Combo")
class LOWPOLY_API UCKAnimNotify_CommitCombo : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};

UCLASS(DisplayName = "CK Reset Combo")
class LOWPOLY_API UCKAnimNotify_ResetCombo : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};

UCLASS(DisplayName = "CK Lunge")
class LOWPOLY_API UCKAnimNotify_Lunge : public UAnimNotify
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    float LungeDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Combat")
    float LungeSpeed = 1000.0f;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
