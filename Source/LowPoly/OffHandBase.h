#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "SkillSlotInfo.h"
#include "WeaponStanceTypes.h"
#include "OffHandBase.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class LOWPOLY_API AOffHandBase : public AItemBase
{
	GENERATED_BODY()

public:
	AOffHandBase();

    virtual void UseOffHand(class APlayerCharacter* Character);
    virtual void StopUseOffHand(class APlayerCharacter* Character);

	virtual FSkillSlotInfo GetSkillInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "CK|Stance")
	EWeaponHoldStance GetHoldStance() const;
	virtual EWeaponHoldStance GetHoldStance_Implementation() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|HUD")
	class UTexture2D* ActionIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|HUD")
	FText ActionName = NSLOCTEXT("Skill", "Use", "Use");

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
