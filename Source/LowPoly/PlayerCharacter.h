#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WeaponStanceTypes.h"
#include "ItemBase.h"
#include "InventoryComponent.h"
#include "LobbyStation.h"
#include "PlayerCharacter.generated.h"

class UAttributeComponent;
class UInventoryComponent;
class UInputMappingContext;
class UInputAction;
class ARangedWeaponBase;
class AMainHandBase;
class UShopContextMenuWidget;
struct FInputActionValue;

UCLASS()
class LOWPOLY_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

    // --- Networking ---
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_AttachItem(AItemBase* Item, FName SocketName);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_DetachItem(AItemBase* Item);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SwapItem(AItemBase* OldItem, AItemBase* NewItem, FName SocketName);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayAttackMontage(class UAnimMontage* MontageToPlay, FName SectionName);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayOffhandAttackMontage(class UAnimMontage* MontageToPlay, FName SectionName);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_StopMontage(class UAnimMontage* MontageToStop, float BlendOutTime = 0.2f);

	// --- Accessors ---
	UFUNCTION(BlueprintCallable, Category = "CK|Equipment") FName GetMainHandSocketName() const { return MainHandSocketName; }
	UFUNCTION(BlueprintCallable, Category = "CK|Equipment") FName GetOffHandSocketName() const { return OffHandSocketName; }
	UFUNCTION(BlueprintCallable, Category = "CK|Interaction") AActor* GetBestInteractable() const;
	UFUNCTION(BlueprintCallable, Category = "CK|Components") UAttributeComponent* GetAttributeComponent() const { return AttributeComponent; }
	UFUNCTION(BlueprintCallable, Category = "CK|Components") UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	UFUNCTION(BlueprintCallable, Category = "CK|Camera") class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

    // --- Shop Logic ---
    UPROPERTY(ReplicatedUsing = OnRep_ShopPool, BlueprintReadOnly, Category = "CK|Shop")
    TArray<FName> ShopPool;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CK|Shop")
    class UDataTable* ShopDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Shop")
    int32 RerollCost = 40;

    UPROPERTY(ReplicatedUsing = OnRep_LockedSlots, BlueprintReadOnly, Category = "CK|Shop")
    TArray<bool> LockedSlots;

    void GenerateShopPool();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "CK|Shop")
    void Server_PurchaseItem(int32 SlotIndex);

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "CK|Shop")
    void Server_ToggleLockSlot(int32 SlotIndex);

    UFUNCTION(Client, Reliable)
    void Client_RefreshShopDisplay();

    UFUNCTION(Client, Reliable)
    void Client_OnDamageDealt();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "CK|Shop")
    void Server_RerollShop();

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Economy")
	void Server_SellItem_Direct(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Economy")
    void Server_SellItem(ESlotGroup SourceGroup, int32 SlotIndex);

    // --- Data Persistence (survives ServerTravel) ---
    /** Saves all character data (stats, gold, inventory, shop) into PlayerState before map travel */
    UFUNCTION(BlueprintCallable, Category = "CK|Persist")
    void SaveToPlayerState();

    /** Restores all character data from PlayerState after arriving on a new map */
    UFUNCTION(BlueprintCallable, Category = "CK|Persist")
    void RestoreFromPlayerState();

    /** Saves all character data to PersistenceManager singleton (survives ServerTravel) */
    UFUNCTION(BlueprintCallable, Category = "CK|Persist")
    void SaveToPersistenceManager();

    /** Restores all character data from PersistenceManager singleton after map travel */
    UFUNCTION(BlueprintCallable, Category = "CK|Persist")
    void RestoreFromPersistenceManager();

    // --- Lobby Logic ---
    UFUNCTION(Client, Reliable)
    void Client_OpenShop();

    UFUNCTION(Client, Reliable)
    void Client_OpenLevelUp();

    UFUNCTION(Client, Reliable)
    void Client_OpenEquip();

    UFUNCTION(BlueprintImplementableEvent, Category = "CK|Lobby")
    void BP_OnOpenLobbyUI(ELobbyStationType Type);

    // --- Weapon/Ability Helpers ---
    AMainHandBase* GetActiveMainHandWeapon() const;
    void StartAttackMontageSection(int32 C);
    UFUNCTION(Server, Reliable) void Server_SetSprinting(bool S);

	// --- Gameplay State ---
	UFUNCTION(BlueprintCallable, Category = "CK|Ready") bool IsReadyForNextRound() const { return bReadyForNextRound; }
	UFUNCTION(BlueprintCallable, Category = "CK|Combat") bool IsAiming() const { return bIsAiming; }
	UFUNCTION(BlueprintCallable, Category = "CK|Stance") EWeaponHoldStance GetHoldStance() const { return CurrentStance; }
    UFUNCTION(BlueprintCallable, Category = "CK|Combat") bool IsBlocking() const { return bIsBlocking; }
    void SetIsBlocking(bool bNewBlocking) { bIsBlocking = bNewBlocking; }

	// --- Modifiers ---
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Modifiers") float CustomMovementMultiplier = 1.0f;
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "CK|Modifiers") float CustomRotationMultiplier = 1.0f;

	void RefreshHoldStance();
	UFUNCTION(Server, Reliable) void Server_SetAiming(bool bAiming);
	UFUNCTION(BlueprintImplementableEvent, Category = "CK|Camera") void BP_OnAimStateChanged(bool bNewAiming);
	UFUNCTION(BlueprintImplementableEvent, Category = "CK|Camera") void BP_OnScopeStateChanged(bool bNewScoping);
	UFUNCTION(BlueprintCallable, Category = "CK|Scope") bool IsScoping() const { return bIsScoping; }
	UFUNCTION(NetMulticast, Reliable) void Multicast_OnAimStateChanged(bool bNewAiming);
	UFUNCTION(Client, Unreliable) void Client_ApplyRecoil(float Pitch, float Yaw);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Debug") void Server_DebugWinRound();
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Debug") void Server_DebugLoseRound();
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "CK|Ready") void Server_SetReadyForNextRound(bool bReady);

	void ToggleLoadout();
    void ToggleCrouch();
    void SetRotationClamp(bool bEnable, float Limit);
    void ToggleEscapeMenu();
    void CloseAllUI();
    void CloseContextMenu();
    
    UPROPERTY() UShopContextMenuWidget* ActiveContextMenuInstance;

    /** Non-null while the player is channeling a spectral shield spell */
    UPROPERTY()
    class ASpectralShieldBase* ActiveSpectralShield = nullptr;

    // --- Input & Combat ---
	UFUNCTION(BlueprintCallable, Category = "CK|Combat") void AnimNotify_OpenComboWindow();
	UFUNCTION(BlueprintCallable, Category = "CK|Combat") void AnimNotify_CloseComboWindow();
	UFUNCTION(BlueprintCallable, Category = "CK|Combat") void AnimNotify_CommitCombo();
	UFUNCTION(BlueprintCallable, Category = "CK|Combat") void AnimNotify_ResetCombo();
    UFUNCTION(BlueprintCallable, Category = "CK|Combat") void AnimNotify_Lunge(float Distance, float Speed);
	UFUNCTION(BlueprintCallable, Category = "CK|Combat") void AnimNotify_MeleeHitWindowBegin();
	UFUNCTION(BlueprintCallable, Category = "CK|Combat") void AnimNotify_MeleeHitWindowTick(float FrameDeltaTime);
	UFUNCTION(BlueprintCallable, Category = "CK|Combat") void AnimNotify_MeleeHitWindowEnd();

    void UpdateInputGating();

    // --- Components & Input Actions ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Components")
	UAttributeComponent* AttributeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Components")
	UInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* InteractAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* AttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputMappingContext* ToggleLoadoutContext; 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* SwapMainHandAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* SwapOffHandAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* OffHandAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* SprintAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* ToggleLoadoutAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* ReloadAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* EscapeMenuAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* ScoreboardAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* Ability1Action;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* Ability2Action;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CK|Input") UInputAction* Ability3Action;

    // --- UI/Camera State ---
	UPROPERTY(EditDefaultsOnly, Category = "CK|UI") TSubclassOf<class UCKEscapeMenuWidget> EscapeMenuWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "CK|UI") TSubclassOf<class UCKScoreboardWidget> ScoreboardWidgetClass;
	UPROPERTY() class UCKEscapeMenuWidget* EscapeMenuInstance;
	UPROPERTY() class UCKScoreboardWidget* ScoreboardInstance;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Camera") class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CK|Camera") class UCameraComponent* FollowCamera;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Camera") float PitchMin = -60.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Camera") float PitchMax = 60.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Camera") float DefaultFOV = 90.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Camera") float AimFOV = 65.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Camera") float AimFOVInterpSpeed = 8.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Camera") FVector AimCameraOffset = FVector(0.f, 60.f, 10.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Camera") float DefaultArmLength = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Camera") float ScopePositionInterpSpeed = 35.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Camera") float ScopeRotationInterpSpeed = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Camera") float ScopedMontagePlayRate = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Camera") bool bScopeStabilized = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Camera") FVector ScopeStabilizedOffset = FVector(20.0f, 0.0f, 150.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Camera") float AimCameraInterpSpeed = 8.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Recoil") float RecoilApplySpeed = 25.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Recoil") float RecoilRecoverySpeed = 8.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Recoil") float RecoilRecoveryDelay = 0.15f;
	UPROPERTY(EditDefaultsOnly, Category = "CK|UI") TSubclassOf<class UUserWidget> LoadoutWidgetClass;
	UPROPERTY() UUserWidget* LoadoutWidgetInstance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Crouch") float CrouchOffset = -50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Crouch") float CrouchInterpSpeed = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Equipment") FName MainHandSocketName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Equipment") FName OffHandSocketName;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Ready") bool bReadyForNextRound;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Combat") bool bIsAiming;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Stance") EWeaponHoldStance CurrentStance = EWeaponHoldStance::Empty;
	UPROPERTY(Replicated) bool bIsRotationClamped;
	UPROPERTY(Replicated) float ClampedYawCenter;
	float YawLimit;
	UPROPERTY(ReplicatedUsing = OnRep_IsScoping, BlueprintReadOnly, Category = "CK|Scope") bool bIsScoping = false;
	bool bIsCrouched = false;
	UPROPERTY() class UCKScopeOverlayWidget* ScopeOverlayInstance;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Combat") float AimPitch;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Combat") float AimYaw;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Combat") FVector AimLookAtLocation;
	float PendingRecoilPitch = 0.0f;
	float PendingRecoilYaw = 0.0f;
	float AccumulatedRecoilPitch = 0.0f;
	float LastRecoilTime = -999.0f;
	bool bCanUseOffHand = true;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Combat") bool bIsAttacking;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Combat") bool bIsComboWindowOpen;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Combat") bool bComboInputQueued;
	bool bIsLunging;
	FVector DefaultCameraOffset;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Combat") bool bIsBlocking;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CK|Combat") int32 CurrentComboIndex;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Input Handlers
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();
	UFUNCTION(Server, Reliable) void Server_Interact();
	void Attack();
	void StopAttack();
	UFUNCTION(Server, Reliable) void Server_Attack();
	UFUNCTION(Server, Reliable) void Server_StopAttack();
	UFUNCTION(Server, Reliable) void Server_AnimNotify_OpenComboWindow();
	UFUNCTION(Server, Reliable) void Server_AnimNotify_CloseComboWindow();
	UFUNCTION(Server, Reliable) void Server_CommitCombo();
	UFUNCTION(Server, Reliable) void Server_ResetCombo();
	UFUNCTION(Server, Reliable) void Server_AnimNotify_MeleeHitWindowBegin();
	UFUNCTION(Server, Reliable) void Server_AnimNotify_MeleeHitWindowTick(float FrameDeltaTime);
	UFUNCTION(Server, Reliable) void Server_AnimNotify_MeleeHitWindowEnd();
	void SwapMainHand();
	void SwapOffHand();
    void OnUseOffHand();
    void OnStopUseOffHand();
public:
	UFUNCTION(Server, Reliable) void Server_SetBlocking(bool bBlocking);
protected:
	UFUNCTION(Server, Reliable) void Server_CastSecondarySpell();
	UFUNCTION(Server, Reliable) void Server_UseActiveOffHand();
	UFUNCTION(Server, Reliable) void Server_StopActiveOffHand();
	void Reload();
	UFUNCTION(Server, Reliable) void Server_Reload();
	void StartSprint();
	void StopSprint();
	void ToggleScoreboard();
	bool IsUIOpen() const;
	void Ability1();
	void Ability2();
	void Ability3();
	UFUNCTION(Server, Reliable) void Server_UseAbility(int32 SlotIndex);
	UFUNCTION(Server, Reliable) void Server_SetScoping(bool bScoping);
	void UpdateScopingState();
	UFUNCTION() void OnRep_IsScoping();
    UFUNCTION() void OnRep_ShopPool();
    UFUNCTION() void OnRep_LockedSlots();
};
