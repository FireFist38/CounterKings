#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeComponent.h"
#include "LevelUpWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class LOWPOLY_API ULevelUpWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- Stats UI Bindings ---
	UPROPERTY(meta = (BindWidget)) UTextBlock* PointsRemainingText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* TimerText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* CurrentLevelText;
	
	UPROPERTY(meta = (BindWidget)) UButton* Health_Plus;
	UPROPERTY(meta = (BindWidget)) UButton* Health_Minus;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Health_PendingText;

	UPROPERTY(meta = (BindWidget)) UButton* Strength_Plus;
	UPROPERTY(meta = (BindWidget)) UButton* Strength_Minus;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Strength_PendingText;

	UPROPERTY(meta = (BindWidget)) UButton* Dexterity_Plus;
	UPROPERTY(meta = (BindWidget)) UButton* Dexterity_Minus;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Dexterity_PendingText;

	UPROPERTY(meta = (BindWidget)) UButton* Magic_Plus;
	UPROPERTY(meta = (BindWidget)) UButton* Magic_Minus;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Magic_PendingText;

	UPROPERTY(meta = (BindWidget)) UButton* Luck_Plus;
	UPROPERTY(meta = (BindWidget)) UButton* Luck_Minus;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Luck_PendingText;

	UPROPERTY(meta = (BindWidget)) UButton* ConfirmButton;
	UPROPERTY(meta = (BindWidget)) UButton* BuyXPButton;
	UPROPERTY(meta = (BindWidget)) class UCKXPBarWidget* XPBar;

	void UpdateUI();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Use raw integers instead of TMap to avoid destructor issues during UObject cleanup
    int32 PendingHealth = 0;
    int32 PendingStrength = 0;
    int32 PendingDexterity = 0;
    int32 PendingMagic = 0;
    int32 PendingLuck = 0;

    int32 CommittedHealth = 0;
    int32 CommittedStrength = 0;
    int32 CommittedDexterity = 0;
    int32 CommittedMagic = 0;
    int32 CommittedLuck = 0;

	UFUNCTION() void AddHealth();
	UFUNCTION() void SubHealth();
	UFUNCTION() void AddStrength();
	UFUNCTION() void SubStrength();
	UFUNCTION() void AddDexterity();
	UFUNCTION() void SubDexterity();
	UFUNCTION() void AddMagic();
	UFUNCTION() void SubMagic();
	UFUNCTION() void AddLuck();
	UFUNCTION() void SubLuck();

	void ModifyPending(EAttributeType Attribute, int32 Amount);
	UFUNCTION() void ConfirmUpgrades();
	UFUNCTION() void OnBuyXPClicked();
};
