#include "PlayerCharacter.h"
#include "CKGameMode.h"
#include "AttributeComponent.h"
#include "InventoryComponent.h"
#include "ItemBase.h"
#include "AbilityBase.h"
#include "MainHandBase.h"
#include "RangedWeaponBase.h"
#include "SecondaryRangedBase.h"
#include "MagicWeaponBase.h"
#include "OffHandBase.h"
#include "SpellBase.h"
#include "SpellChannelBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "CKGameUserSettings.h"
#include "CKSettingsWidget.h"
#include "CKEscapeMenuWidget.h"
#include "CKScoreboardWidget.h"
#include "CKScopeOverlayWidget.h"
#include "Components/SceneCaptureComponent2D.h"
#include "LootTableEntry.h"
#include "Engine/DataTable.h"
#include "CKGameState.h"
#include "ShopWidget.h"
#include "ShopContextMenuWidget.h"
#include "GameFramework/HUD.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "CKHUD.h"
#include "CKHUDWidget.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);
    DefaultCameraOffset = CameraBoom->SocketOffset;
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	bReadyForNextRound = false;
	MainHandSocketName = FName("MainHandSocket");
	OffHandSocketName = FName("OffHandSocket");
	bIsAttacking = false;
	bIsComboWindowOpen = false;
	bComboInputQueued = false;
	bIsLunging = false;
	bIsAiming = false;
    bIsScoping = false;
	CurrentComboIndex = 0;
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false; 
    bUseControllerRotationRoll = false;
    LockedSlots.Init(false, 5);
    ActiveContextMenuInstance = nullptr;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
	if (APlayerController* PC = Cast<APlayerController>(Controller)) { if (PC->PlayerCameraManager) { PC->PlayerCameraManager->ViewPitchMin = PitchMin; PC->PlayerCameraManager->ViewPitchMax = PitchMax; } }
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller)) { if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) { Subsystem->AddMappingContext(DefaultMappingContext, 0); } }
    UpdateInputGating();
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    // Character Orientation Logic (For ADS and Hip-Fire Alignment)
    if (IsLocallyControlled() || HasAuthority())
    {
        FVector CamLoc;
        FRotator CamRot;
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            PC->GetPlayerViewPoint(CamLoc, CamRot);
        }
        else
        {
            CamLoc = FollowCamera->GetComponentLocation();
            CamRot = FollowCamera->GetComponentRotation();
        }

        // Replicated look-at point for animation/system use
        AimLookAtLocation = CamLoc + (CamRot.Vector() * 10000.0f);
        
        // Force character to face the camera direction when Aiming OR Attacking (Hip-fire)
        if (bIsAiming || bIsAttacking)
        {
            // Manually set the actor rotation to match the camera yaw for perfect alignment
            FRotator NewRotation = GetActorRotation();
            NewRotation.Yaw = CamRot.Yaw;
            SetActorRotation(NewRotation);

            bUseControllerRotationYaw = true;
            if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
            {
                MoveComp->bOrientRotationToMovement = false;
            }
        }
        else
        {
            bUseControllerRotationYaw = false;
            if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
            {
                MoveComp->bOrientRotationToMovement = true;
            }
        }

        // Relative aim offsets (Pitch/Yaw) normalized for AnimBP (calculated AFTER manual rotation update)
        FRotator Delta = (CamRot - GetActorRotation()).GetNormalized();
        AimPitch = Delta.Pitch;
        AimYaw = Delta.Yaw;
    }

    if (Controller)
    {
        if (!FMath::IsNearlyZero(PendingRecoilPitch) || !FMath::IsNearlyZero(PendingRecoilYaw))
        {
            const float PitchStep = FMath::FInterpTo(0.0f, PendingRecoilPitch, DeltaTime, RecoilApplySpeed);
            const float YawStep = FMath::FInterpTo(0.0f, PendingRecoilYaw, DeltaTime, RecoilApplySpeed);
            AddControllerPitchInput(-PitchStep);
            AddControllerYawInput(YawStep);
            PendingRecoilPitch -= PitchStep;
            PendingRecoilYaw -= YawStep;
        }
    }

    if (FollowCamera && CameraBoom)
    {
        float TargetFOV = DefaultFOV;
        float TargetArmLength = DefaultArmLength;
        FVector TargetSocketOffset = DefaultCameraOffset;
        float CameraInterpSpeed = AimCameraInterpSpeed;

        if (bIsScoping)
        {
            if (ARangedWeaponBase* R = Cast<ARangedWeaponBase>(GetActiveMainHandWeapon()))
            {
                TargetFOV = R->GetScopedFOV();
                TargetArmLength = R->GetScopedArmLength();
                TargetSocketOffset = bScopeStabilized ? ScopeStabilizedOffset : R->GetScopedCameraOffset();
                CameraInterpSpeed = ScopePositionInterpSpeed;
            }
        }
        else if (bIsAiming)
        {
            TargetFOV = AimFOV;
            TargetArmLength = DefaultArmLength;
            TargetSocketOffset = AimCameraOffset;
        }

        if (bIsCrouched)
        {
            TargetSocketOffset.Z += CrouchOffset;
        }

        FollowCamera->FieldOfView = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaTime, AimFOVInterpSpeed);
        CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, CameraInterpSpeed);
        CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, CameraInterpSpeed);

        if (bIsScoping)
        {
            if (ARangedWeaponBase* R = Cast<ARangedWeaponBase>(GetActiveMainHandWeapon()))
            {
                if (USceneCaptureComponent2D* PIPCapture = R->GetPIPCapture())
                {
                    PIPCapture->SetWorldRotation(GetControlRotation());
                }
            }
        }
    }
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PC) 
{ 
    Super::SetupPlayerInputComponent(PC); 
    if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PC))
    {
        EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
        EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
        EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayerCharacter::Interact);
        if (AttackAction) { EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &APlayerCharacter::Attack); EIC->BindAction(AttackAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopAttack); }
        if (EscapeMenuAction) { EIC->BindAction(EscapeMenuAction, ETriggerEvent::Started, this, &APlayerCharacter::ToggleEscapeMenu); }
        EIC->BindAction(SwapMainHandAction, ETriggerEvent::Started, this, &APlayerCharacter::SwapMainHand);
        EIC->BindAction(SwapOffHandAction, ETriggerEvent::Started, this, &APlayerCharacter::SwapOffHand);
        EIC->BindAction(OffHandAction, ETriggerEvent::Started, this, &APlayerCharacter::OnUseOffHand);
        EIC->BindAction(OffHandAction, ETriggerEvent::Completed, this, &APlayerCharacter::OnStopUseOffHand);
        EIC->BindAction(OffHandAction, ETriggerEvent::Canceled, this, &APlayerCharacter::OnStopUseOffHand);
        EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &APlayerCharacter::StartSprint);
        EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopSprint);
        EIC->BindAction(ToggleLoadoutAction, ETriggerEvent::Started, this, &APlayerCharacter::ToggleLoadout);
        if (ScoreboardAction) { EIC->BindAction(ScoreboardAction, ETriggerEvent::Started, this, &APlayerCharacter::ToggleScoreboard); }
        if (CrouchAction) { EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &APlayerCharacter::ToggleCrouch); }
        if (ReloadAction) EIC->BindAction(ReloadAction, ETriggerEvent::Started, this, &APlayerCharacter::Reload);
        if (Ability1Action) EIC->BindAction(Ability1Action, ETriggerEvent::Started, this, &APlayerCharacter::Ability1);
        if (Ability2Action) EIC->BindAction(Ability2Action, ETriggerEvent::Started, this, &APlayerCharacter::Ability2);
        if (Ability3Action) EIC->BindAction(Ability3Action, ETriggerEvent::Started, this, &APlayerCharacter::Ability3);
    }
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
    if (bIsAttacking || IsUIOpen()) return;
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(ForwardDirection, MovementVector.Y * CustomMovementMultiplier);
		AddMovementInput(RightDirection, MovementVector.X * CustomMovementMultiplier);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
    if (IsUIOpen()) return;
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		float Sensitivity = 1.0f;
		if (UCKGameUserSettings* Settings = Cast<UCKGameUserSettings>(GEngine->GetGameUserSettings()))
         { 
            Sensitivity = Settings->GetMouseSensitivity(); 
            if (bIsScoping) Sensitivity *= Settings->GetAimSensitivityMultiplier() * 0.5f;
            else if (bIsAiming) Sensitivity *= Settings->GetAimSensitivityMultiplier(); 
         }
        float FinalYaw = LookAxisVector.X * Sensitivity * CustomRotationMultiplier;
        float FinalPitch = LookAxisVector.Y * Sensitivity * CustomRotationMultiplier;
        if (bIsRotationClamped)
        {
            FRotator CurrentRot = Controller->GetControlRotation();
            float NewYaw = CurrentRot.Yaw + FinalYaw;
            float NormalizedNewYaw = FRotator::NormalizeAxis(NewYaw);
            float NormalizedCenter = FRotator::NormalizeAxis(ClampedYawCenter);
            float Delta = FRotator::NormalizeAxis(NormalizedNewYaw - NormalizedCenter);
            float ClampedDelta = FMath::Clamp(Delta, -YawLimit, YawLimit);
            float FinalYawClamped = FRotator::NormalizeAxis(NormalizedCenter + ClampedDelta);
            Controller->SetControlRotation(FRotator(FMath::ClampAngle(CurrentRot.Pitch + FinalPitch, PitchMin, PitchMax), FinalYawClamped, 0.0f));
        }
        else
        {
		    AddControllerYawInput(FinalYaw);
		    AddControllerPitchInput(FinalPitch);
        }
	}
}

void APlayerCharacter::GenerateShopPool()
{
    if (!HasAuthority() || !ShopDataTable) return;
    if (ShopPool.Num() != 5) ShopPool.Init(NAME_None, 5);
    if (LockedSlots.Num() != 5) LockedSlots.Init(false, 5);
    TArray<FName> RowNames = ShopDataTable->GetRowNames();
    for (int32 i = 0; i < 5; i++)
    {
        if (!LockedSlots[i])
        {
            if (RowNames.Num() > 0) ShopPool[i] = RowNames[FMath::RandRange(0, RowNames.Num() - 1)];
            else ShopPool[i] = NAME_None;
        }
    }
}

void APlayerCharacter::Server_PurchaseItem_Implementation(int32 SlotIndex)
{
    if (!HasAuthority() || !ShopPool.IsValidIndex(SlotIndex) || ShopPool[SlotIndex].IsNone()) return;
    if (!AttributeComponent || !InventoryComponent || !ShopDataTable) return;
    FLootTableEntry* Entry = ShopDataTable->FindRow<FLootTableEntry>(ShopPool[SlotIndex], TEXT("Purchase"));
    if (!Entry || !Entry->ItemClass) return;
    if (AttributeComponent->GetGold() >= Entry->BuyPrice)
    {
        AttributeComponent->AddGold(-Entry->BuyPrice);
        AItemBase* NewItem = GetWorld()->SpawnActor<AItemBase>(Entry->ItemClass, GetActorLocation(), FRotator::ZeroRotator);
        if (NewItem)
        {
            NewItem->SetItemName(FName(*Entry->ItemName.ToString()));
            NewItem->SetDescription(Entry->Description);
            NewItem->SetRarity(Entry->Rarity);
            NewItem->SetGoldValue(Entry->SellPrice);
            InventoryComponent->Server_TryPickupItem(NewItem);
            ShopPool[SlotIndex] = NAME_None;
            if (LockedSlots.IsValidIndex(SlotIndex)) LockedSlots[SlotIndex] = false;
            Client_RefreshShopDisplay();
        }
    }
}

void APlayerCharacter::Server_ToggleLockSlot_Implementation(int32 SlotIndex)
{
    if (HasAuthority() && LockedSlots.IsValidIndex(SlotIndex))
    {
        bool bWasLocked = LockedSlots[SlotIndex];
        LockedSlots.Init(false, 5);
        if (!bWasLocked) LockedSlots[SlotIndex] = true;
    }
}

void APlayerCharacter::Client_RefreshShopDisplay_Implementation()
{
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UShopWidget::StaticClass(), false);
    for (UUserWidget* Widget : FoundWidgets) if (UShopWidget* ShopWidget = Cast<UShopWidget>(Widget)) ShopWidget->RefreshShopDisplay();
}

void APlayerCharacter::Server_RerollShop_Implementation()
{
    if (AttributeComponent && AttributeComponent->GetGold() >= RerollCost)
    {
        AttributeComponent->AddGold(-RerollCost);
        GenerateShopPool();
        Client_RefreshShopDisplay();
    }
}

void APlayerCharacter::Server_SellItem_Direct_Implementation(int32 SlotIndex)
{
    if (!HasAuthority() || !InventoryComponent || !AttributeComponent) return;
    AItemBase* ItemToSell = InventoryComponent->GetBagItem(SlotIndex);
    if (!ItemToSell) return;
    AttributeComponent->AddGold(ItemToSell->GetGoldValue());
    InventoryComponent->RemoveItem(ItemToSell);
    ItemToSell->Destroy();
}

void APlayerCharacter::CloseContextMenu()
{
    if (ActiveContextMenuInstance && ActiveContextMenuInstance->IsInViewport())
    {
        ActiveContextMenuInstance->RemoveFromParent();
        ActiveContextMenuInstance = nullptr;
    }
}

void APlayerCharacter::CloseAllUI()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    if (EscapeMenuInstance && EscapeMenuInstance->IsInViewport()) EscapeMenuInstance->CloseMenu();
    if (LoadoutWidgetInstance && LoadoutWidgetInstance->IsInViewport()) LoadoutWidgetInstance->RemoveFromParent();
    if (ScoreboardInstance && ScoreboardInstance->IsInViewport()) { ScoreboardInstance->RemoveFromParent(); ScoreboardInstance = nullptr; }
    
    CloseContextMenu();

    PC->SetInputMode(FInputModeGameOnly());
    PC->bShowMouseCursor = false;
}

void APlayerCharacter::ToggleScoreboard()
{
    if (!IsLocallyControlled() || !ScoreboardWidgetClass) return;
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        if (ScoreboardInstance && ScoreboardInstance->IsInViewport())
        {
            ScoreboardInstance->RemoveFromParent();
            ScoreboardInstance = nullptr;
            PC->SetInputMode(FInputModeGameOnly());
            PC->bShowMouseCursor = false;
        }
        else
        {
            CloseAllUI();
            ScoreboardInstance = CreateWidget<UCKScoreboardWidget>(PC, ScoreboardWidgetClass);
            if (ScoreboardInstance)
            {
                ScoreboardInstance->AddToViewport();
                PC->SetInputMode(FInputModeGameAndUI().SetWidgetToFocus(ScoreboardInstance->TakeWidget()));
                PC->bShowMouseCursor = true;
            }
        }
    }
}

bool APlayerCharacter::IsUIOpen() const
{
    if (EscapeMenuInstance && EscapeMenuInstance->IsInViewport()) return true;
    if (LoadoutWidgetInstance && LoadoutWidgetInstance->IsInViewport()) return true;
    if (ScoreboardInstance && ScoreboardInstance->IsInViewport()) return true;
    if (ActiveContextMenuInstance && ActiveContextMenuInstance->IsInViewport()) return true;
    return false;
}

void APlayerCharacter::UpdateInputGating() { bCanUseOffHand = !(GetActiveMainHandWeapon() && GetActiveMainHandWeapon()->IsTwoHanded()); }

void APlayerCharacter::SetRotationClamp(bool bEnable, float Limit)
{
    if (!HasAuthority()) return;
    bIsRotationClamped = bEnable;
    YawLimit = Limit;
    if (bEnable && Controller) ClampedYawCenter = Controller->GetControlRotation().Yaw;
}

void APlayerCharacter::Interact()
{
	if (!HasAuthority()) { Server_Interact(); return; }
	AItemBase* BestItem = GetBestInteractable();
	if (BestItem && InventoryComponent) InventoryComponent->Server_TryPickupItem(BestItem);
}

AItemBase* APlayerCharacter::GetBestInteractable() const
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, AItemBase::StaticClass());
	AItemBase* BestItem = nullptr;
	float MinDistSq = 500.0f * 500.0f;
	for (AActor* Actor : OverlappingActors)
	{
		AItemBase* Item = Cast<AItemBase>(Actor);
		if (Item && !Item->IsHidden()) { float DistSq = FVector::DistSquared(GetActorLocation(), Item->GetActorLocation()); if (DistSq < MinDistSq) { MinDistSq = DistSq; BestItem = Item; } }
	}
	return BestItem;
}

void APlayerCharacter::Multicast_AttachItem_Implementation(AItemBase* Item, FName SocketName)
{
    if (Item) Item->Equip(Cast<USceneComponent>(GetMesh()), SocketName);
    RefreshHoldStance();
    UpdateInputGating();
    UpdateScopingState();
}

void APlayerCharacter::Multicast_DetachItem_Implementation(AItemBase* Item)
{
    if (Item) Item->Unequip();
    RefreshHoldStance();
    UpdateInputGating();
    UpdateScopingState();
}

void APlayerCharacter::Multicast_SwapItem_Implementation(AItemBase* Old, AItemBase* New, FName S) 
{ 
    if(Old) Old->Unequip(); 
    if(New) New->Equip(Cast<USceneComponent>(GetMesh()), S); 
    RefreshHoldStance();
    UpdateInputGating();
    UpdateScopingState();
}

void APlayerCharacter::Multicast_PlayAttackMontage_Implementation(UAnimMontage* M, FName S) { if (USkeletalMeshComponent* SMesh = GetMesh()) if (UAnimInstance* AI = SMesh->GetAnimInstance()) { AI->Montage_Play(M); if(S!=NAME_None) AI->Montage_JumpToSection(S, M); } }
void APlayerCharacter::Multicast_PlayOffhandAttackMontage_Implementation(UAnimMontage* M, FName S) { if (USkeletalMeshComponent* SMesh = GetMesh()) if (UAnimInstance* AI = SMesh->GetAnimInstance()) { AI->Montage_Play(M); if(S!=NAME_None) AI->Montage_JumpToSection(S, M); } }
void APlayerCharacter::Multicast_StopMontage_Implementation(UAnimMontage* M, float B) { if (USkeletalMeshComponent* SMesh = GetMesh()) if (UAnimInstance* AI = SMesh->GetAnimInstance()) AI->Montage_Stop(B, M); }
void APlayerCharacter::Server_Interact_Implementation() { Interact(); }

void APlayerCharacter::Attack() { if (IsUIOpen()) return; bIsAttacking = true; Server_Attack(); }
void APlayerCharacter::StopAttack() { bIsAttacking = false; Server_StopAttack(); }

void APlayerCharacter::RefreshHoldStance() 
{ 
    if (HasAuthority() && InventoryComponent) 
    { 
        EWeaponHoldStance S = EWeaponHoldStance::Empty; 
        if (AMainHandBase* M = Cast<AMainHandBase>(InventoryComponent->GetActiveMainHandItem())) S = M->GetHoldStance(); 
        CurrentStance = S; 
    } 
}

void APlayerCharacter::ToggleEscapeMenu() 
{ 
    if (!IsLocallyControlled() || !EscapeMenuWidgetClass) return; 
    APlayerController* PC = Cast<APlayerController>(GetController()); 
    if (PC) 
    { 
        if (EscapeMenuInstance && EscapeMenuInstance->IsInViewport()) 
        { 
            EscapeMenuInstance->CloseMenu(); 
            CloseContextMenu();
            PC->SetInputMode(FInputModeGameOnly()); 
            PC->bShowMouseCursor = false;
        } 
        else 
        { 
            CloseAllUI();
            EscapeMenuInstance = CreateWidget<UCKEscapeMenuWidget>(PC, EscapeMenuWidgetClass); 
            if (EscapeMenuInstance) 
            { 
                EscapeMenuInstance->AddToViewport(); 
                FInputModeGameAndUI Mode; 
                Mode.SetWidgetToFocus(EscapeMenuInstance->TakeWidget()); 
                PC->SetInputMode(Mode); 
                PC->bShowMouseCursor = true; 
            } 
        } 
    } 
}

void APlayerCharacter::Client_ApplyRecoil_Implementation(float P, float Y) { PendingRecoilPitch += P; PendingRecoilYaw += Y; LastRecoilTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f; }

void APlayerCharacter::Server_StopAttack_Implementation() { bIsAttacking = false; if (ARangedWeaponBase* R = Cast<ARangedWeaponBase>(GetActiveMainHandWeapon())) R->StopAutoFire(); if (AMagicWeaponBase* M = Cast<AMagicWeaponBase>(GetActiveMainHandWeapon())) M->StopPrimaryChannel(); }

void APlayerCharacter::Server_Attack_Implementation() 
{ 
    if (AttributeComponent && AttributeComponent->IsDead()) return; 
    if (bIsBlocking) return; 
    AMainHandBase* W = GetActiveMainHandWeapon(); 
    if (!W) return; 
    
    if (W->IsRanged()) 
    { 
        bIsAttacking = true;
        if (ARangedWeaponBase* R = Cast<ARangedWeaponBase>(W)) { if (R->IsAutomatic()) R->StartAutoFire(this); else R->Fire(this); } 
        return; 
    } 
    if (AMagicWeaponBase* M = Cast<AMagicWeaponBase>(W)) 
    { 
        bIsAttacking = true;
        if (Cast<USpellChannelBase>(M->GetPrimarySpell())) M->StartPrimaryChannel(this); 
        else M->CastPrimary(this); 
        return; 
    } 

    // MELEE LOGIC: Check bIsComboWindowOpen instead of just bIsAttacking
    // because bIsAttacking is now set locally in Attack() for rotation prediction.
    if (bIsComboWindowOpen) 
    { 
        bComboInputQueued = true; 
        return; 
    } 
    
    // Only start a new attack if we aren't currently playing a montage section
    if (USkeletalMeshComponent* SMesh = GetMesh())
    {
        if (UAnimInstance* AI = SMesh->GetAnimInstance())
        {
            if (AI->Montage_IsPlaying(W->GetAttackMontage()))
            {
                // We are already attacking but the combo window isn't open yet.
                return;
            }
        }
    }
    CurrentComboIndex = 0; bIsAttacking = true; StartAttackMontageSection(0); 
}

void APlayerCharacter::AnimNotify_OpenComboWindow() { if (HasAuthority()) Server_AnimNotify_OpenComboWindow_Implementation(); else Server_AnimNotify_OpenComboWindow(); }
void APlayerCharacter::AnimNotify_CloseComboWindow() { if (HasAuthority()) Server_AnimNotify_CloseComboWindow_Implementation(); else Server_AnimNotify_CloseComboWindow(); }
void APlayerCharacter::AnimNotify_CommitCombo() { if (HasAuthority()) Server_CommitCombo_Implementation(); else Server_CommitCombo(); }
void APlayerCharacter::AnimNotify_ResetCombo() { if (HasAuthority()) Server_ResetCombo_Implementation(); else Server_ResetCombo(); }

void APlayerCharacter::AnimNotify_Lunge(float D, float S) 
{ 
    if (UCharacterMovementComponent* M = GetCharacterMovement()) 
    { 
        FVector F = GetActorForwardVector(); M->Velocity = F * S; M->SetMovementMode(MOVE_Flying); 
        FTimerHandle H; GetWorldTimerManager().SetTimer(H, [M](){ if (M) M->SetMovementMode(MOVE_Walking); }, 0.2f, false); 
    } 
}

void APlayerCharacter::AnimNotify_MeleeHitWindowBegin() { if (HasAuthority()) Server_AnimNotify_MeleeHitWindowBegin_Implementation(); else Server_AnimNotify_MeleeHitWindowBegin(); }
void APlayerCharacter::AnimNotify_MeleeHitWindowTick(float D) { if (HasAuthority()) Server_AnimNotify_MeleeHitWindowTick_Implementation(D); else Server_AnimNotify_MeleeHitWindowTick(D); }
void APlayerCharacter::AnimNotify_MeleeHitWindowEnd() { if (HasAuthority()) Server_AnimNotify_MeleeHitWindowEnd_Implementation(); else Server_AnimNotify_MeleeHitWindowEnd(); }

void APlayerCharacter::Server_AnimNotify_OpenComboWindow_Implementation() { bIsComboWindowOpen = true; }
void APlayerCharacter::Server_AnimNotify_CloseComboWindow_Implementation() { bIsComboWindowOpen = false; }

void APlayerCharacter::Server_CommitCombo_Implementation() 
{ 
    AMainHandBase* W = GetActiveMainHandWeapon(); 
    if (!W) { Server_ResetCombo_Implementation(); return; } 
    const int32 Next = CurrentComboIndex + 1; 
    if (bComboInputQueued && W->GetComboSectionName(Next) != NAME_None) 
    { 
        CurrentComboIndex = Next; bComboInputQueued = false; bIsComboWindowOpen = false; StartAttackMontageSection(CurrentComboIndex); 
        return; 
    } 
    Server_ResetCombo_Implementation(); 
}

void APlayerCharacter::Server_ResetCombo_Implementation() { bIsAttacking = false; bIsComboWindowOpen = false; bComboInputQueued = false; bIsLunging = false; CurrentComboIndex = 0; }
void APlayerCharacter::Server_AnimNotify_MeleeHitWindowBegin_Implementation() { if (AMainHandBase* W = GetActiveMainHandWeapon()) W->BeginMeleeHitWindow(this); }
void APlayerCharacter::Server_AnimNotify_MeleeHitWindowTick_Implementation(float D) { if (AMainHandBase* W = GetActiveMainHandWeapon()) W->TickMeleeHitWindow(this); }
void APlayerCharacter::Server_AnimNotify_MeleeHitWindowEnd_Implementation() { if (AMainHandBase* W = GetActiveMainHandWeapon()) W->EndMeleeHitWindow(); }

void APlayerCharacter::StartAttackMontageSection(int32 C) 
{ 
    AMainHandBase* W = GetActiveMainHandWeapon(); 
    if (!W || !W->GetAttackMontage()) { Server_ResetCombo_Implementation(); return; } 
    if (AttributeComponent) 
    { 
        if (W->bUseMana) { if (!AttributeComponent->ConsumeMana(W->ManaCost)) { Server_ResetCombo_Implementation(); return; } } 
        else { if (!AttributeComponent->ConsumeStamina(W->StaminaCost)) { Server_ResetCombo_Implementation(); return; } } 
    } else { Server_ResetCombo_Implementation(); return; } 
    const FName S = W->GetComboSectionName(C); if (S == NAME_None) { Server_ResetCombo_Implementation(); return; } 
    Multicast_PlayAttackMontage(W->GetAttackMontage(), S); 
}

AMainHandBase* APlayerCharacter::GetActiveMainHandWeapon() const { return InventoryComponent ? Cast<AMainHandBase>(InventoryComponent->GetActiveMainHandItem()) : nullptr; }
void APlayerCharacter::SwapMainHand() { if (InventoryComponent) InventoryComponent->Server_CycleMainHand(); }
void APlayerCharacter::SwapOffHand() { if (InventoryComponent) InventoryComponent->Server_CycleOffHand(); }

void APlayerCharacter::OnUseOffHand() 
{ 
	if (bCanUseOffHand) 
	{ 
		AOffHandBase* O = InventoryComponent ? Cast<AOffHandBase>(InventoryComponent->GetActiveOffHandItem()) : nullptr; 
		if (ASecondaryRangedBase* R = Cast<ASecondaryRangedBase>(O)) { bIsAttacking = true; R->UseOffHand(this); return; } 
	} 
	AMainHandBase* M = GetActiveMainHandWeapon();
	if (M && M->IsRanged()) 
    {
        // Prediction for snappy camera
        bIsAiming = true;
        Server_SetAiming(true);
    }
	else if (AMagicWeaponBase* M2 = Cast<AMagicWeaponBase>(M)) { bIsAttacking = true; Server_CastSecondarySpell(); }
	else Server_SetBlocking(true);
}

void APlayerCharacter::OnStopUseOffHand() 
{ 
	if (bCanUseOffHand) 
	{ 
		AOffHandBase* O = InventoryComponent ? Cast<AOffHandBase>(InventoryComponent->GetActiveOffHandItem()) : nullptr; 
		if (ASecondaryRangedBase* R = Cast<ASecondaryRangedBase>(O)) { bIsAttacking = false; R->StopUseOffHand(this); return; } 
	}
	AMainHandBase* M = GetActiveMainHandWeapon();
	if (M && M->IsRanged()) 
    {
        bIsAiming = false;
        Server_SetAiming(false);
    }
	else if (AMagicWeaponBase* M2 = Cast<AMagicWeaponBase>(M)) { bIsAttacking = false; if (Cast<USpellChannelBase>(M2->GetSecondarySpell())) Server_SetBlocking(false); return; }
	else Server_SetBlocking(false);
}

void APlayerCharacter::Server_CastSecondarySpell_Implementation() { bIsAttacking = true; if (AttributeComponent && AttributeComponent->IsDead()) return; if (AMagicWeaponBase* M = Cast<AMagicWeaponBase>(GetActiveMainHandWeapon())) { if (Cast<USpellChannelBase>(M->GetSecondarySpell())) M->StartSecondaryChannel(this); else M->CastSecondary(this); } }
void APlayerCharacter::Server_UseActiveOffHand_Implementation() { if (AttributeComponent && AttributeComponent->IsDead()) return; if (!InventoryComponent) return; if (AOffHandBase* O = Cast<AOffHandBase>(InventoryComponent->GetActiveOffHandItem())) O->UseOffHand(this); }
void APlayerCharacter::Server_StopActiveOffHand_Implementation() { if (!InventoryComponent) return; if (AOffHandBase* O = Cast<AOffHandBase>(InventoryComponent->GetActiveOffHandItem())) O->StopUseOffHand(this); }

void APlayerCharacter::Server_SetAiming_Implementation(bool bAiming) 
{ 
    ARangedWeaponBase* R = Cast<ARangedWeaponBase>(GetActiveMainHandWeapon()); 
    if (bAiming && R && R->IsCyclingBolt()) return; 
    bIsAiming = bAiming; 
    if (R) 
    { 
        UAnimMontage* M = R->GetAimMontage(); 
        if (M) { if (bAiming) Multicast_PlayAttackMontage(M, NAME_None); else Multicast_StopMontage(M, 0.2f); } 
    } 
    Multicast_OnAimStateChanged(bAiming); UpdateScopingState();
}

void APlayerCharacter::Server_SetScoping_Implementation(bool bScoping) { if (bIsScoping == bScoping) return; bIsScoping = bScoping; if (IsLocallyControlled()) OnRep_IsScoping(); }

void APlayerCharacter::UpdateScopingState() 
{ 
    bool bShouldScope = false; if (bIsAiming) if (ARangedWeaponBase* R = Cast<ARangedWeaponBase>(GetActiveMainHandWeapon())) if (R->IsScoped()) bShouldScope = true;
    if (HasAuthority()) Server_SetScoping_Implementation(bShouldScope); else Server_SetScoping(bShouldScope);
}

void APlayerCharacter::OnRep_IsScoping()
{
    if (IsLocallyControlled())
    {
        if (bIsScoping)
        {
            if (ARangedWeaponBase* R = Cast<ARangedWeaponBase>(GetActiveMainHandWeapon()))
            {
                if (TSubclassOf<UCKScopeOverlayWidget> OverlayClass = R->GetScopeOverlayClass())
                {
                    if (ScopeOverlayInstance) ScopeOverlayInstance->RemoveFromParent();
                    ScopeOverlayInstance = CreateWidget<UCKScopeOverlayWidget>(GetWorld(), OverlayClass);
                    if (ScopeOverlayInstance) ScopeOverlayInstance->AddToViewport();
                }
            }
        }
        else { if (ScopeOverlayInstance) { ScopeOverlayInstance->RemoveFromParent(); ScopeOverlayInstance = nullptr; } }
    }
    BP_OnScopeStateChanged(bIsScoping);
    if (ARangedWeaponBase* R = Cast<ARangedWeaponBase>(GetActiveMainHandWeapon())) R->SetPIPCaptureEnabled(IsLocallyControlled() && bIsScoping);
}

void APlayerCharacter::Multicast_OnAimStateChanged_Implementation(bool B) { BP_OnAimStateChanged(B); }
void APlayerCharacter::Reload() { Server_Reload(); }
void APlayerCharacter::Server_Reload_Implementation() { if (ARangedWeaponBase* M = Cast<ARangedWeaponBase>(GetActiveMainHandWeapon())) M->StartReload(this); else if (ASecondaryRangedBase* O = Cast<ASecondaryRangedBase>(InventoryComponent->GetActiveOffHandItem())) O->StartReload(this); }
void APlayerCharacter::Server_SetBlocking_Implementation(bool B) { if (!B) if (AMagicWeaponBase* M = Cast<AMagicWeaponBase>(GetActiveMainHandWeapon())) if (Cast<USpellChannelBase>(M->GetSecondarySpell())) { M->StopSecondaryChannel(); return; } if (!InventoryComponent) return; AOffHandBase* O = Cast<AOffHandBase>(InventoryComponent->GetActiveOffHandItem()); if (!O) return; if (B && bIsAttacking) return; if (B) O->UseOffHand(this); else O->StopUseOffHand(this); }
void APlayerCharacter::StartSprint() { Server_SetSprinting(true); }
void APlayerCharacter::StopSprint() { Server_SetSprinting(false); }

void APlayerCharacter::ToggleLoadout() 
{ 
    if (!IsLocallyControlled() || !LoadoutWidgetClass) return; 
    APlayerController* PC = Cast<APlayerController>(GetController()); 
    if (PC) 
    { 
        if (LoadoutWidgetInstance && LoadoutWidgetInstance->IsInViewport()) 
        { 
            LoadoutWidgetInstance->RemoveFromParent(); 
            CloseContextMenu();
            PC->SetInputMode(FInputModeGameOnly()); 
            PC->bShowMouseCursor = false; 
        } 
        else 
        { 
            CloseAllUI();
            LoadoutWidgetInstance = CreateWidget<UUserWidget>(PC, LoadoutWidgetClass); 
            if (LoadoutWidgetInstance) 
            { 
                LoadoutWidgetInstance->AddToViewport(); 
                PC->SetInputMode(FInputModeGameAndUI().SetWidgetToFocus(LoadoutWidgetInstance->TakeWidget())); 
                PC->bShowMouseCursor = true; 
            } 
        } 
    } 
}

void APlayerCharacter::Server_DebugWinRound_Implementation() { if (HasAuthority()) if (ACKGameMode* GM = Cast<ACKGameMode>(GetWorld()->GetAuthGameMode())) GM->StartPostRoundPhaseWithResult(true); }
void APlayerCharacter::Server_DebugLoseRound_Implementation() { if (HasAuthority()) if (ACKGameMode* GM = Cast<ACKGameMode>(GetWorld()->GetAuthGameMode())) GM->StartPostRoundPhaseWithResult(false); }
void APlayerCharacter::Server_SetReadyForNextRound_Implementation(bool bReady) { bReadyForNextRound = bReady; }

void APlayerCharacter::ToggleCrouch() { bIsCrouched = !bIsCrouched; }
void APlayerCharacter::Server_SetSprinting_Implementation(bool S) { if (AttributeComponent) AttributeComponent->SetSprinting(S); }
void APlayerCharacter::Ability1() { Server_UseAbility(0); }
void APlayerCharacter::Ability2() { Server_UseAbility(1); }
void APlayerCharacter::Ability3() { Server_UseAbility(2); }
void APlayerCharacter::Server_UseAbility_Implementation(int32 SlotIndex) { if ((AttributeComponent && AttributeComponent->IsDead()) || IsUIOpen()) return; if (!InventoryComponent) return; if (InventoryComponent->IsAbilitySlotUnlocked(SlotIndex)) { TArray<AAbilityBase*> Abilities = InventoryComponent->GetAbilitySlots(); if (Abilities.IsValidIndex(SlotIndex) && Abilities[SlotIndex]) Abilities[SlotIndex]->ActivateAbility(this); } }

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{ 
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); 
    DOREPLIFETIME(APlayerCharacter, bReadyForNextRound); 
    DOREPLIFETIME(APlayerCharacter, bIsAiming); 
    DOREPLIFETIME(APlayerCharacter, bIsScoping); 
    DOREPLIFETIME(APlayerCharacter, bIsAttacking); 
    DOREPLIFETIME(APlayerCharacter, bIsComboWindowOpen); 
    DOREPLIFETIME(APlayerCharacter, bComboInputQueued); 
    DOREPLIFETIME(APlayerCharacter, bIsBlocking); 
    DOREPLIFETIME(APlayerCharacter, CurrentComboIndex); 
    DOREPLIFETIME(APlayerCharacter, CurrentStance); 
    DOREPLIFETIME(APlayerCharacter, ShopPool); 
    DOREPLIFETIME(APlayerCharacter, LockedSlots); 
    DOREPLIFETIME(APlayerCharacter, bIsRotationClamped); 
    DOREPLIFETIME(APlayerCharacter, ClampedYawCenter); 
    DOREPLIFETIME(APlayerCharacter, CustomMovementMultiplier); 
    DOREPLIFETIME(APlayerCharacter, CustomRotationMultiplier); 
    DOREPLIFETIME(APlayerCharacter, AimPitch);
    DOREPLIFETIME(APlayerCharacter, AimYaw);
    DOREPLIFETIME(APlayerCharacter, AimLookAtLocation);
}

void APlayerCharacter::Client_OnDamageDealt_Implementation() 
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (ACKHUD* HUD = Cast<ACKHUD>(PC->GetHUD()))
        {
            if (UCKHUDWidget* HUDWidget = Cast<UCKHUDWidget>(HUD->GetHUDWidget()))
            {
                HUDWidget->ShowHitmarker();
            }
        }
    }
}
