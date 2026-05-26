#include "CKHUDWidget.h"
#include "PreRoundRootWidget.h"
#include "CKRoundResultWidget.h"
#include "CKXPBarWidget.h"
#include "CKPlayerState.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "InventoryComponent.h"
#include "CKGameState.h"
#include "ItemBase.h"
#include "AbilityBase.h"
#include "LootTableEntry.h"
#include "RangedWeaponBase.h"
#include "SecondaryRangedBase.h"
#include "MagicWeaponBase.h"
#include "MainHandBase.h"
#include "OffHandBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void UCKHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateStatus();
	UpdateInventoryIcons();

	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (Character && InteractionPrompt)
	{
		bool bInRange = Character->GetBestInteractable() != nullptr;
		InteractionPrompt->SetVisibility(bInRange ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	UpdateRangedHUD(InDeltaTime);
	UpdateSkillSlots();

	ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (GS)
	{
		// Phase change detection
		if (GS->GetMatchPhase() != PreviousPhase)
		{
			if (GS->GetMatchPhase() == ECKMatchPhase::PostRound)
			{
				ShowRoundResult(GS->bDebugWin);
			}
			else if (GS->GetMatchPhase() == ECKMatchPhase::Combat)
			{
				HideAllPostRoundUI();
			}
			PreviousPhase = GS->GetMatchPhase();
		}

		if (RoundText) RoundText->SetText(FText::AsNumber(GS->GetCurrentRound()));
		if (TimerText) TimerText->SetText(FText::AsNumber(FMath::CeilToInt(GS->GetRemainingTime())));
		UpdateAbilityGating(GS->GetCurrentRound());
	}
}

void UCKHUDWidget::ShowRoundResult(bool bWin)
{
	TSubclassOf<UCKRoundResultWidget> WidgetToSpawn = bWin ? VictoryWidgetClass : DefeatWidgetClass;
	if (!WidgetToSpawn)
	{
		// If no result widget, jump straight to shop
		ShowPreRoundMenu();
		return;
	}

	ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
	int32 RoundNum = GS ? GS->GetCurrentRound() : 0;

	APlayerController* PC = GetOwningPlayer();
	ResultWidgetInstance = CreateWidget<UCKRoundResultWidget>(PC, WidgetToSpawn);
	if (ResultWidgetInstance)
	{
		ResultWidgetInstance->SetupResult(bWin, RoundNum);
		ResultWidgetInstance->AddToViewport(200);
		
		// Set timer to start the fade out slightly before the menu swap
		float FadeStartTime = FMath::Max(0.1f, ResultDisplayDuration - 1.0f);

		// USE WEAK OBJECT POINTERS TO PREVENT CRASH IF WIDGET IS DESTROYED
		TWeakObjectPtr<UCKRoundResultWidget> WeakResult = ResultWidgetInstance;
		FTimerHandle FadeTimer;
		GetWorld()->GetTimerManager().SetTimer(FadeTimer, [WeakResult]()
        {
            if (WeakResult.IsValid())
            {
                WeakResult->PlayFadeOut();
            }
        }, FadeStartTime, false);

		// Set timer to swap to the shop menu
		FTimerHandle ResultTimer;
		GetWorld()->GetTimerManager().SetTimer(ResultTimer, this, &UCKHUDWidget::ShowPreRoundMenu, ResultDisplayDuration, false);
	}
}

void UCKHUDWidget::ShowPreRoundMenu()
{
	// Clean up the result screen
	if (ResultWidgetInstance)
	{
		ResultWidgetInstance->RemoveFromParent();
		ResultWidgetInstance = nullptr;
	}

	// Only show the menu if we are still in the PostRound phase
	ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (GS && GS->GetMatchPhase() == ECKMatchPhase::PostRound)
	{
		if (PreRoundRootWidgetClass && !PreRoundRootInstance)
		{
			APlayerController* PC = GetOwningPlayer();
            if (PC)
            {
			    PreRoundRootInstance = CreateWidget<UPreRoundRootWidget>(PC, PreRoundRootWidgetClass);
			    if (PreRoundRootInstance)
			    {
				    PreRoundRootInstance->AddToViewport(100);
				    PC->SetShowMouseCursor(true);
				    PC->SetInputMode(FInputModeGameAndUI().SetWidgetToFocus(PreRoundRootInstance->TakeWidget()));
			    }
            }
		}
	}
}

void UCKHUDWidget::HideAllPostRoundUI()
{
	if (ResultWidgetInstance)
	{
		ResultWidgetInstance->RemoveFromParent();
		ResultWidgetInstance = nullptr;
	}

	if (PreRoundRootInstance)
	{
		PreRoundRootInstance->RemoveFromParent();
		PreRoundRootInstance = nullptr;

		if (APlayerController* PC = GetOwningPlayer())
		{
			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
}

void UCKHUDWidget::UpdateStatus()
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    ACKPlayerState* PS = GetOwningPlayerState<ACKPlayerState>();

	if (Character && Character->GetAttributeComponent())
	{
		UAttributeComponent* Attr = Character->GetAttributeComponent();
		
		if (HealthBar) HealthBar->SetPercent(Attr->GetMaxHealth() > 0 ? Attr->GetCurrentHealth() / Attr->GetMaxHealth() : 0);
		if (StaminaBar) 
		{
			StaminaBar->SetPercent(Attr->GetMaxStamina() > 0 ? Attr->GetCurrentStamina() / Attr->GetMaxStamina() : 0);
		}
		if (ManaBar)
		{
			ManaBar->SetPercent(Attr->GetMaxMana() > 0 ? Attr->GetCurrentMana() / Attr->GetMaxMana() : 0);
		}

        // Update Match HP
        if (PS)
        {
            if (PlayerMatchHPBar) PlayerMatchHPBar->SetPercent(PS->MatchHealth / 100.0f);
            if (PlayerHealthValue) PlayerHealthValue->SetText(FText::AsNumber(PS->MatchHealth));
            if (PlayerNameText) PlayerNameText->SetText(FText::FromString(PS->GetPlayerName()));
        }
        else
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Red, TEXT("PS NULL in UpdateStatus"));
        }

		if (GoldText) GoldText->SetText(FText::AsNumber(Attr->GetGold()));
        if (CurrentLevelText) CurrentLevelText->SetText(FText::AsNumber(Attr->GetLevel()));
        
        // Update XP Bar
        if (XPBar) XPBar->UpdateXPBar(Attr->GetLevel(), Attr->GetCurrentXP(), Attr->GetXPThreshold());
	}

    // --- Opponent Placeholder ---
    if (OpponentNameText) OpponentNameText->SetText(FText::FromString(TEXT("Opponent")));
    if (OpponentMatchHPBar) OpponentMatchHPBar->SetPercent(0.5f);
    if (OpponentHealthValue) OpponentHealthValue->SetText(FText::AsNumber(50));
}

void UCKHUDWidget::UpdateInventoryIcons()
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (!Character || !Character->GetInventoryComponent()) return;

	UInventoryComponent* Inv = Character->GetInventoryComponent();
	ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!GS || !GS->GetLootDataTable()) return;

	auto UpdateSlot = [&](UImage* SlotImg, AItemBase* Item)
	{
		if (SlotImg)
		{
			if (Item)
			{
				TArray<FLootTableEntry*> Rows;
				GS->GetLootDataTable()->GetAllRows<FLootTableEntry>(TEXT("IconLookup"), Rows);
				for(auto* Row : Rows)
				{
					if (Row->ItemClass == Item->GetClass())
					{
						if (Row->ItemIcon)
						{
							SlotImg->SetBrushFromTexture(Row->ItemIcon);
							SlotImg->SetOpacity(1.0f);
							return;
						}
					}
				}
			}
			SlotImg->SetOpacity(0.0f);
		}
	};

	UpdateSlot(MainHandIcon, Inv->GetActiveMainHandItem());
	UpdateSlot(OffHandIcon, Inv->GetActiveOffHandItem());
	UpdateSlot(ConsumableIcon, Inv->GetConsumableSlot());

    // Update Abilities
    TArray<AAbilityBase*> Abilities = Inv->GetAbilitySlots();
    TArray<UImage*> AbilityImages = { Ability_0, Ability_1, Ability_2 };
    for (int32 i = 0; i < 3; i++)
    {
        if (AbilityImages.IsValidIndex(i))
        {
            UpdateSlot(AbilityImages[i], Abilities.IsValidIndex(i) ? Abilities[i] : nullptr);
        }
    }
}

void UCKHUDWidget::UpdateAbilityGating(int32 CurrentRound)
{
    APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (!Character || !Character->GetInventoryComponent()) return;

	UInventoryComponent* Inv = Character->GetInventoryComponent();

	if (Ability_0) Ability_0->SetOpacity(Inv->IsAbilitySlotUnlocked(0) ? 1.0f : 0.2f);
	if (Ability_1) Ability_1->SetOpacity(Inv->IsAbilitySlotUnlocked(1) ? 1.0f : 0.2f);
	if (Ability_2) Ability_2->SetOpacity(Inv->IsAbilitySlotUnlocked(2) ? 1.0f : 0.2f);
}

void UCKHUDWidget::UpdateRangedHUD(float DeltaTime)
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (!Character || !Character->GetInventoryComponent()) return;

	AItemBase* MainItem = Character->GetInventoryComponent()->GetActiveMainHandItem();
	AItemBase* OffItem = Character->GetInventoryComponent()->GetActiveOffHandItem();

	ARangedWeaponBase* MainRanged = Cast<ARangedWeaponBase>(MainItem);
	ASecondaryRangedBase* OffRanged = Cast<ASecondaryRangedBase>(OffItem);

	if (Crosshair)
	{
		AItemBase* CrosshairSourceItem = nullptr;
		if (MainItem && MainItem->ShouldShowCrosshair())
		{
			CrosshairSourceItem = MainItem;
		}
		else if (OffItem && OffItem->ShouldShowCrosshair())
		{
			CrosshairSourceItem = OffItem;
		}

		const bool bShow = CrosshairSourceItem && !Character->IsScoping();
		Crosshair->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

		if (bShow)
		{
			const ECrosshairStyle Style = CrosshairSourceItem->GetCrosshairStyle();
			UTexture2D* StyleTexture = DefaultCrosshairTexture;
			float StyleScaleMultiplier = 1.0f;

			switch (Style)
			{
			case ECrosshairStyle::Shotgun:
				if (ShotgunCrosshairTexture) StyleTexture = ShotgunCrosshairTexture;
				StyleScaleMultiplier = 1.25f;
				break;
			case ECrosshairStyle::Bow:
				if (BowCrosshairTexture) StyleTexture = BowCrosshairTexture;
				StyleScaleMultiplier = 0.90f;
				break;
			case ECrosshairStyle::Precision:
				if (PrecisionCrosshairTexture) StyleTexture = PrecisionCrosshairTexture;
				StyleScaleMultiplier = 0.80f;
				break;
			case ECrosshairStyle::Magic:
				if (MagicCrosshairTexture) StyleTexture = MagicCrosshairTexture;
				StyleScaleMultiplier = 1.05f;
				break;
			case ECrosshairStyle::Offhand:
				if (OffhandCrosshairTexture) StyleTexture = OffhandCrosshairTexture;
				StyleScaleMultiplier = 1.10f;
				break;
			case ECrosshairStyle::Unique:
				if (UniqueCrosshairTexture) StyleTexture = UniqueCrosshairTexture;
				StyleScaleMultiplier = 1.0f;
				break;
			default:
				if (DefaultCrosshairTexture) StyleTexture = DefaultCrosshairTexture;
				StyleScaleMultiplier = 1.0f;
				break;
			}

			if (StyleTexture && StyleTexture != LastAppliedCrosshairTexture)
			{
				Crosshair->SetBrushFromTexture(StyleTexture);
				LastAppliedCrosshairTexture = StyleTexture;
			}

			const float BaseScale = CrosshairHipBaseScale * (Character->IsAiming() ? CrosshairAimScaleMultiplier : 1.0f) * StyleScaleMultiplier;
			const float FireBloomAlpha = MainRanged ? MainRanged->GetCrosshairBloomAlpha() : (OffRanged ? OffRanged->GetCrosshairBloomAlpha() : 0.0f);
			const float MovementSpeed = Character->GetVelocity().Size2D();
			const float MoveBloomAlpha = FMath::Clamp(MovementSpeed / FMath::Max(CrosshairMovementSpeedForMaxBloom, 1.0f), 0.0f, 1.0f) * CrosshairMovementBloomMaxAlpha;
			const float CombinedBloomAlpha = FMath::Clamp(FireBloomAlpha + MoveBloomAlpha, 0.0f, 1.0f);
			const float TargetScale = BaseScale * (1.0f + CombinedBloomAlpha * CrosshairBloomMaxScaleAdd);

			CurrentCrosshairScale = FMath::FInterpTo(CurrentCrosshairScale, TargetScale, DeltaTime, CrosshairScaleInterpSpeed);
			Crosshair->SetRenderScale(FVector2D(CurrentCrosshairScale, CurrentCrosshairScale));
		}
	}

	if (AmmoText)
	{
		if (MainRanged)
		{
			AmmoText->SetText(FText::Format(
				NSLOCTEXT("HUD", "AmmoFmt", "{0} / {1}"),
				FText::AsNumber(MainRanged->GetCurrentAmmo()),
				FText::AsNumber(MainRanged->GetMagazineCapacity())
			));
			AmmoText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else if (OffRanged)
		{
			AmmoText->SetText(FText::Format(
				NSLOCTEXT("HUD", "AmmoFmt", "{0} / {1}"),
				FText::AsNumber(OffRanged->GetCurrentAmmo()),
				FText::AsNumber(OffRanged->GetMagazineCapacity())
			));
			AmmoText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			AmmoText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (ReloadBar)
	{
		bool bIsReloading = (MainRanged && MainRanged->IsReloading()) || (OffRanged && OffRanged->IsReloading());
		float Progress = 0.0f;
		if (MainRanged && MainRanged->IsReloading()) Progress = MainRanged->GetReloadProgress();
		else if (OffRanged && OffRanged->IsReloading()) Progress = OffRanged->GetReloadProgress();

		if (bIsReloading)
		{
			ReloadBar->SetPercent(Progress);
			ReloadBar->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			ReloadBar->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (BoltActionBar)
	{
		if (MainRanged && MainRanged->IsCyclingBolt())
		{
			BoltActionBar->SetPercent(MainRanged->GetBoltProgress());
			BoltActionBar->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			BoltActionBar->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

static void ApplySlot(UImage* IconImg, UProgressBar* CooldownBar, const FSkillSlotInfo& Info)
{
	if (IconImg)
	{
		if (Info.bIsValid)
		{
			if (Info.Icon)
				IconImg->SetBrushFromTexture(Info.Icon);
			IconImg->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			IconImg->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (CooldownBar)
	{
		const bool bShow = Info.bIsValid && Info.bHasCooldown && Info.CooldownProgress < 1.0f;
		if (bShow)
		{
			CooldownBar->SetPercent(1.0f - Info.CooldownProgress);
			CooldownBar->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			CooldownBar->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UCKHUDWidget::UpdateSkillSlots()
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (!Character || !Character->GetInventoryComponent()) return;

	AMainHandBase* MainHand = Cast<AMainHandBase>(Character->GetInventoryComponent()->GetActiveMainHandItem());
	AOffHandBase* OffHand = Cast<AOffHandBase>(Character->GetInventoryComponent()->GetActiveOffHandItem());

	FSkillSlotInfo LMB;
	FSkillSlotInfo RMB;

	if (MainHand)
	{
		LMB = MainHand->GetLMBSkillInfo();
		RMB = MainHand->GetRMBSkillInfo();
	}

	if (!RMB.bIsValid && OffHand && !Character->GetInventoryComponent()->IsOffHandLocked())
	{
		RMB = OffHand->GetSkillInfo();
	}

	ApplySlot(LMBIcon, LMBCooldown, LMB);
	ApplySlot(RMBIcon, RMBCooldown, RMB);
}

void UCKHUDWidget::ShowHitmarker()
{
    if (HitmarkerImage)
    {
        HitmarkerImage->SetVisibility(ESlateVisibility::HitTestInvisible);
        GetWorld()->GetTimerManager().ClearTimer(HitmarkerTimerHandle);
        GetWorld()->GetTimerManager().SetTimer(HitmarkerTimerHandle, this, &UCKHUDWidget::HideHitmarker, HitmarkerDuration, false);
    }
}

void UCKHUDWidget::HideHitmarker()
{
    if (HitmarkerImage)
    {
        HitmarkerImage->SetVisibility(ESlateVisibility::Collapsed);
    }
}
