#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKGameState.h"
#include "ItemBase.h"
#include "CKHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class UTexture2D;

UCLASS()
class LOWPOLY_API UCKHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- GDD 7.1 Top Left ---
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ManaBar;

    // --- Tournament Match HP UI ---
    UPROPERTY(meta = (BindWidget))
    UProgressBar* PlayerMatchHPBar;
    
    UPROPERTY(meta = (BindWidget))
    UProgressBar* OpponentMatchHPBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* OpponentNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerHealthValue;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* OpponentHealthValue;

	// --- GDD 7.1 Top Right ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GoldText;

	// --- GDD 7.1 Top Center ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoundText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimerText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentLevelText;

	// --- GDD 7.1 Bottom Slots (Revised) ---
	UPROPERTY(meta = (BindWidget))
	UImage* MainHandIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* OffHandIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* Ability_0;
	UPROPERTY(meta = (BindWidget))
	UImage* Ability_1;
	UPROPERTY(meta = (BindWidget))
	UImage* Ability_2;

	UPROPERTY(meta = (BindWidget))
	UImage* ConsumableIcon;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* InteractionPrompt;

	// --- Ranged HUD ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ReloadBar;

	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* BoltActionBar;

	// --- Skill slots ---
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* LMBIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* LMBCooldown;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* RMBIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* RMBCooldown;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Crosshair;

    // --- Hitmarker ---
    UPROPERTY(meta = (BindWidgetOptional))
    UImage* HitmarkerImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|UI")
    float HitmarkerDuration = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Crosshair")
	UTexture2D* DefaultCrosshairTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Crosshair")
	UTexture2D* ShotgunCrosshairTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Crosshair")
	UTexture2D* BowCrosshairTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Crosshair")
	UTexture2D* PrecisionCrosshairTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Crosshair")
	UTexture2D* MagicCrosshairTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Crosshair")
	UTexture2D* OffhandCrosshairTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Crosshair")
	UTexture2D* UniqueCrosshairTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Crosshair")
	float CrosshairHipBaseScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Crosshair")
	float CrosshairAimScaleMultiplier = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Crosshair")
	float CrosshairBloomMaxScaleAdd = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Crosshair")
	float CrosshairMovementBloomMaxAlpha = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Crosshair")
	float CrosshairMovementSpeedForMaxBloom = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CK|Crosshair")
	float CrosshairScaleInterpSpeed = 12.0f;

	UPROPERTY(meta = (BindWidgetOptional))
	class UCKXPBarWidget* XPBar;

	UPROPERTY(EditDefaultsOnly, Category = "CK|UI")
	TSubclassOf<class UCKRoundResultWidget> VictoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "CK|UI")
	TSubclassOf<class UCKRoundResultWidget> DefeatWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "CK|UI")
	float ResultDisplayDuration = 5.0f;

	UPROPERTY()
	UCKRoundResultWidget* ResultWidgetInstance;

	UPROPERTY(EditDefaultsOnly, Category = "CK|UI")
	TSubclassOf<class UPreRoundRootWidget> PreRoundRootWidgetClass;

	UPROPERTY()
	class UPreRoundRootWidget* PreRoundRootInstance;

    UFUNCTION(BlueprintCallable, Category = "CK|UI")
    void ShowHitmarker();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	ECKMatchPhase PreviousPhase = ECKMatchPhase::Waiting;

	void ShowRoundResult(bool bWin);
	void ShowPreRoundMenu();
	void HideAllPostRoundUI();

	void UpdateStatus();
	void UpdateInventoryIcons();
	void UpdateAbilityGating(int32 CurrentRound);
	void UpdateRangedHUD(float DeltaTime);
	void UpdateSkillSlots();

	float CurrentCrosshairScale = 1.0f;
	UTexture2D* LastAppliedCrosshairTexture = nullptr;

    FTimerHandle HitmarkerTimerHandle;
    void HideHitmarker();
};
