#include "AttributeComponent.h"
#include "Net/UnrealNetwork.h"
#include "CKGameMode.h"
#include "CKGameState.h"
#include "PlayerCharacter.h"
#include "InventoryComponent.h"
#include "OffHandBase.h"
#include "ShieldBase.h"
#include "SpectralShieldBase.h"
#include "PerkBase.h"
#include "FloatingDamageActor.h"
#include "Kismet/GameplayStatics.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	MaxHealth = 100.0f;
	CurrentHealth = 100.0f;
	bIsDead = false;
	MaxStamina = 100.0f;
	CurrentStamina = 100.0f;
	bIsSprinting = false;
	bIsExhausted = false;
	LastStaminaChangeTime = 0.0f;
	ExhaustionEndTime = 0.0f;
	Strength = 0.0f;
	Dexterity = 0.0f;
	Magic = 0.0f;
	Luck = 0.0f;
	Gold = 0;
	AttributePoints = 0;
	FloatingDamageActorClass = AFloatingDamageActor::StaticClass();
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		CurrentHealth = MaxHealth;
		CurrentStamina = MaxStamina;
		CurrentMana = MaxMana;
	}
}

void UAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAttributeComponent, MaxHealth);
	DOREPLIFETIME(UAttributeComponent, CurrentHealth);
	DOREPLIFETIME(UAttributeComponent, bIsDead);
	DOREPLIFETIME(UAttributeComponent, MaxStamina);
	DOREPLIFETIME(UAttributeComponent, CurrentStamina);
	DOREPLIFETIME(UAttributeComponent, bIsExhausted);
	DOREPLIFETIME(UAttributeComponent, MaxMana);
	DOREPLIFETIME(UAttributeComponent, CurrentMana);
	DOREPLIFETIME(UAttributeComponent, Level);
	DOREPLIFETIME(UAttributeComponent, CurrentXP);
	DOREPLIFETIME(UAttributeComponent, Strength);
	DOREPLIFETIME(UAttributeComponent, Dexterity);
	DOREPLIFETIME(UAttributeComponent, Magic);
	DOREPLIFETIME(UAttributeComponent, Luck);
	DOREPLIFETIME(UAttributeComponent, Gold);
	DOREPLIFETIME(UAttributeComponent, AttributePoints);
}

void UAttributeComponent::OnRep_MaxHealth()
{
}

void UAttributeComponent::OnRep_AttributePoints()
{
}

void UAttributeComponent::SetSprinting(bool bNewSprinting)
{
	bIsSprinting = bNewSprinting;
}

bool UAttributeComponent::ConsumeStamina(float Amount)
{
    if (GetOwnerRole() != ROLE_Authority) return false;
    if (Amount <= 0.0f) return true;
    if (CurrentStamina < Amount) return false;

    CurrentStamina -= Amount;
    LastStaminaChangeTime = GetWorld()->GetTimeSeconds();
    return true;
}

void UAttributeComponent::RestoreStamina(float Amount)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() != ROLE_Authority || bIsDead) return;

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	// Exhaustion logic
	if (bIsExhausted)
	{
		if (CurrentTime >= ExhaustionEndTime)
		{
			bIsExhausted = false;
		}
	}

	// Blocking stamina drain — continuous drain while holding a physical shield up
	// (Spectral shield uses mana drain instead, handled by the spell channel system)
	APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwner());
	const bool bIsBlocking = PC && PC->IsBlocking() && !PC->ActiveSpectralShield;
	if (bIsBlocking && !bIsExhausted)
	{
		if (CurrentStamina > 0.0f)
		{
			// Use half the sprint drain rate for blocking (passive hold cost)
			CurrentStamina = FMath::Max(0.0f, CurrentStamina - (StaminaDrainRate * 0.5f * DeltaTime));
			LastStaminaChangeTime = CurrentTime;

			if (CurrentStamina <= 0.0f)
			{
				bIsExhausted = true;
				ExhaustionEndTime = CurrentTime + ExhaustionDuration;
				PC->SetIsBlocking(false);
				PC->Server_SetBlocking(false);
			}
		}
	}

	// Sprinting / Regen logic
	if (bIsSprinting && !bIsExhausted)
	{
		if (CurrentStamina > 0.0f)
		{
			CurrentStamina = FMath::Max(0.0f, CurrentStamina - (StaminaDrainRate * DeltaTime));
			LastStaminaChangeTime = CurrentTime;

			if (CurrentStamina <= 0.0f)
			{
				bIsExhausted = true;
				ExhaustionEndTime = CurrentTime + ExhaustionDuration;
				bIsSprinting = false;
			}
		}
		else
		{
			bIsSprinting = false;
		}
	}
	else if (!bIsBlocking)
	{
		if (CurrentTime >= LastStaminaChangeTime + RegenDelay && CurrentStamina < MaxStamina)
		{
			CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (StaminaRegenRate * DeltaTime));
		}
	}

	// Mana regen — authoritative, always on
	if (GetOwnerRole() == ROLE_Authority && CurrentMana < MaxMana && ManaRegenRate > 0.0f)
	{
		CurrentMana = FMath::Min(MaxMana, CurrentMana + ManaRegenRate * DeltaTime);
	}
}

bool UAttributeComponent::ConsumeMana(float Amount)
{
	if (GetOwnerRole() != ROLE_Authority) return false;
	if (Amount <= 0.0f) return true;
	if (CurrentMana < Amount) return false;
	CurrentMana -= Amount;
	return true;
}

void UAttributeComponent::RestoreMana(float Amount)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	CurrentMana = FMath::Clamp(CurrentMana + Amount, 0.0f, MaxMana);
}

void UAttributeComponent::Server_PurchaseXP_Implementation()
{
	if (GetOwnerRole() != ROLE_Authority) return;

	const int32 XP_COST = 100;
	if (Gold >= XP_COST)
	{
		Gold -= XP_COST;
		AddXP(5.0f);
	}
}

void UAttributeComponent::Server_UpgradeAttribute_Implementation(EAttributeType Attribute)
{
	if (AttributePoints <= 0) return;

	switch (Attribute)
	{
	case EAttributeType::Health: MaxHealth += 20.0f; break;
	case EAttributeType::Strength: Strength += 1.0f; break;
	case EAttributeType::Dexterity: Dexterity += 1.0f; break;
	case EAttributeType::Magic: Magic += 1.0f; break;
	case EAttributeType::Luck: Luck += 1.0f; break;
	}

	AttributePoints--;
}

void UAttributeComponent::Server_CommitAttributeUpgrades_Implementation(int32 HealthPoints, int32 StrengthPoints, int32 DexterityPoints, int32 MagicPoints, int32 LuckPoints)
{
	HealthPoints = FMath::Max(0, HealthPoints);
	StrengthPoints = FMath::Max(0, StrengthPoints);
	DexterityPoints = FMath::Max(0, DexterityPoints);
	MagicPoints = FMath::Max(0, MagicPoints);
	LuckPoints = FMath::Max(0, LuckPoints);

	const int32 TotalRequested = HealthPoints + StrengthPoints + DexterityPoints + MagicPoints + LuckPoints;
	if (TotalRequested <= 0 || TotalRequested > AttributePoints)
	{
		return;
	}

	MaxHealth += 20.0f * HealthPoints;
	CurrentHealth = FMath::Min(CurrentHealth + (20.0f * HealthPoints), MaxHealth);
	Strength += StrengthPoints;
	Dexterity += DexterityPoints;
	Magic += MagicPoints;
	Luck += LuckPoints;
	AttributePoints -= TotalRequested;
}

void UAttributeComponent::AddGold(int32 Amount)
{
	if (GetOwnerRole() == ROLE_Authority) Gold += Amount;
}

void UAttributeComponent::AddAttributePoints(int32 Points)
{
	if (GetOwnerRole() == ROLE_Authority) 
	{
		AttributePoints += Points;
	}
}

void UAttributeComponent::AddXP(float Amount)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	CurrentXP += Amount;

	// Level up loop
	while (CurrentXP >= GetXPThreshold())
	{
		CurrentXP -= GetXPThreshold();
		Level++;
		AddAttributePoints(1);
	}
}

void UAttributeComponent::SetLevel(int32 NewLevel)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Level = NewLevel;
	}
}

void UAttributeComponent::SetCurrentXP(float NewXP)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		CurrentXP = NewXP;
	}
}

static EDamageType GetDominantDamageType(const FDamageBundle& Damage)
{
	EDamageType DominantType = EDamageType::Physical;
	float MaxDamage = Damage.Physical;

	if (Damage.Magic > MaxDamage) { MaxDamage = Damage.Magic; DominantType = EDamageType::Magic; }
	if (Damage.Fire > MaxDamage) { MaxDamage = Damage.Fire; DominantType = EDamageType::Fire; }
	if (Damage.Lightning > MaxDamage) { MaxDamage = Damage.Lightning; DominantType = EDamageType::Lightning; }
	if (Damage.Frost > MaxDamage) { MaxDamage = Damage.Frost; DominantType = EDamageType::Frost; }
	if (Damage.Poison > MaxDamage) { MaxDamage = Damage.Poison; DominantType = EDamageType::Poison; }
	if (Damage.Holy > MaxDamage) { MaxDamage = Damage.Holy; DominantType = EDamageType::Holy; }
	if (Damage.Earth > MaxDamage) { MaxDamage = Damage.Earth; DominantType = EDamageType::Earth; }

	return DominantType;
}

bool UAttributeComponent::ApplyCombatDamage(const FDamageBundle& Damage, AActor* DamageInstigator)
{
	if (GetOwnerRole() != ROLE_Authority || bIsDead)
	{
		return false;
	}

    // --- Safe Zone Check ---
    if (UWorld* World = GetWorld())
    {
        if (ACKGameState* GS = World->GetGameState<ACKGameState>())
        {
            if (GS->GetMatchPhase() != ECKMatchPhase::Combat)
            {
                // Damage is only allowed during the Combat phase
                return false;
            }
        }
    }

    FDamageBundle FinalDamage = Damage;

    // --- Process Attacker Perks ---
    if (APlayerCharacter* AttackerPC = Cast<APlayerCharacter>(DamageInstigator))
    {
        if (UInventoryComponent* Inv = AttackerPC->GetInventoryComponent())
        {
            for (APerkBase* Perk : Inv->GetPerkSlots())
            {
                if (Perk)
                {
                    Perk->ModifyOutgoingDamage(FinalDamage, GetOwner());
                }
            }
        }
    }

    // --- Process Defender Perks ---
    APlayerCharacter* DefenderPC = Cast<APlayerCharacter>(GetOwner());
    if (DefenderPC)
    {
        if (UInventoryComponent* Inv = DefenderPC->GetInventoryComponent())
        {
            for (APerkBase* Perk : Inv->GetPerkSlots())
            {
                if (Perk)
                {
                    Perk->ModifyIncomingDamage(FinalDamage, DamageInstigator);
                }
            }
        }

        // --- Blocking Logic ---
        if (DefenderPC->IsBlocking())
        {
            // Priority 1: Spectral Shield (magical barrier) — absorbs damage via mana drain
            if (DefenderPC->ActiveSpectralShield && DefenderPC->ActiveSpectralShield->AbsorbDamage(DamageInstigator, FinalDamage))
            {
                // Spectral shield absorbed the hit — zero out all damage
                FinalDamage.Physical = 0.0f;
                FinalDamage.Magic = 0.0f;
                FinalDamage.Fire = 0.0f;
                FinalDamage.Lightning = 0.0f;
                FinalDamage.Frost = 0.0f;
                FinalDamage.Poison = 0.0f;
                FinalDamage.Holy = 0.0f;
                FinalDamage.Earth = 0.0f;
            }
            // Priority 2: Physical off-hand shield
            else if (UInventoryComponent* Inv = DefenderPC->GetInventoryComponent())
            {
                if (AOffHandBase* OffHand = Cast<AOffHandBase>(Inv->GetActiveOffHandItem()))
                {
                    if (AShieldBase* Shield = Cast<AShieldBase>(OffHand))
                    {
                        // Apply Negation
                        FinalDamage.Physical *= (1.0f - Shield->GetPhysicalNegation());
                        FinalDamage.Magic    *= (1.0f - Shield->GetMagicNegation());
                        FinalDamage.Fire      *= (1.0f - Shield->GetFireNegation());
                        FinalDamage.Lightning *= (1.0f - Shield->GetLightningNegation());
                        FinalDamage.Frost     *= (1.0f - Shield->GetFrostNegation());
                        FinalDamage.Poison    *= (1.0f - Shield->GetPoisonNegation());
                        FinalDamage.Holy      *= (1.0f - Shield->GetHolyNegation());
                        FinalDamage.Earth     *= (1.0f - Shield->GetEarthNegation());

                        // Consume Stamina — if the player lacks stamina, kick them out of blocking
                        if (!ConsumeStamina(Shield->GetStaminaCostOnBlock()))
                        {
                            bIsExhausted = true;
                            ExhaustionEndTime = GetWorld()->GetTimeSeconds() + ExhaustionDuration;
                            if (DefenderPC)
                            {
                                DefenderPC->SetIsBlocking(false);
                                DefenderPC->Server_SetBlocking(false);
                            }
                        }
                        else
                        {
                            Shield->Multicast_OnBlocked();
                        }
                    }
                }
            }
        }
    }

    const float TotalDamage = FinalDamage.GetTotal();
    if (TotalDamage <= 0.0f) return false;

	const EDamageType DominantType = GetDominantDamageType(FinalDamage);
	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - TotalDamage, 0.0f, MaxHealth);
	bIsDead = CurrentHealth <= 0.0f;

	const float AppliedDamage = OldHealth - CurrentHealth;
	if (AppliedDamage > 0.0f)
	{
		Multicast_SpawnFloatingDamage(AppliedDamage, DominantType);

        // Notify instigator for hitmarker
        if (APlayerCharacter* InstigatorPC = Cast<APlayerCharacter>(DamageInstigator))
        {
            InstigatorPC->Client_OnDamageDealt();
        }
	}

	return AppliedDamage > 0.0f;
}

void UAttributeComponent::Multicast_SpawnFloatingDamage_Implementation(float DamageAmount, EDamageType DamageType)
{
	if (GetNetMode() == NM_DedicatedServer || !GetWorld() || !GetOwner() || !FloatingDamageActorClass || DamageAmount <= 0.0f)
	{
		return;
	}

	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	const FVector RandomOffset(FMath::FRandRange(-FloatingDamageSpawnJitterXY, FloatingDamageSpawnJitterXY), FMath::FRandRange(-FloatingDamageSpawnJitterXY, FloatingDamageSpawnJitterXY), FloatingDamageSpawnZOffset);
	const FVector SpawnLocation = OwnerLocation + RandomOffset;

	AFloatingDamageActor* FloatingDamage = GetWorld()->SpawnActor<AFloatingDamageActor>(FloatingDamageActorClass, SpawnLocation, FRotator::ZeroRotator);
	if (FloatingDamage)
	{
		FloatingDamage->InitializeDamage(DamageAmount, DamageType);
	}
}

void UAttributeComponent::RestoreHealth(float HealAmount)
{
	if (GetOwnerRole() != ROLE_Authority || HealAmount <= 0.0f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
	bIsDead = CurrentHealth <= 0.0f;
}

void UAttributeComponent::ApplyStrengthBonus(float Delta)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (FMath::IsNearlyZero(Delta)) return;
	Strength += Delta;
}

void UAttributeComponent::RemoveStrengthBonus(float Delta)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (FMath::IsNearlyZero(Delta)) return;
	Strength -= Delta;
}

void UAttributeComponent::ApplyDexterityBonus(float Delta)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (FMath::IsNearlyZero(Delta)) return;
	Dexterity += Delta;
}

void UAttributeComponent::RemoveDexterityBonus(float Delta)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (FMath::IsNearlyZero(Delta)) return;
	Dexterity -= Delta;
}

void UAttributeComponent::ApplyMagicBonus(float Delta)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (FMath::IsNearlyZero(Delta)) return;
	Magic += Delta;
}

void UAttributeComponent::RemoveMagicBonus(float Delta)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (FMath::IsNearlyZero(Delta)) return;
	Magic -= Delta;
}

void UAttributeComponent::ApplyLuckBonus(float Delta)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (FMath::IsNearlyZero(Delta)) return;
	Luck += Delta;
}

void UAttributeComponent::RemoveLuckBonus(float Delta)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (FMath::IsNearlyZero(Delta)) return;
	Luck -= Delta;
}

void UAttributeComponent::ApplyHealthBonus(float Delta)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (FMath::IsNearlyZero(Delta)) return;

	MaxHealth += Delta;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
	bIsDead = CurrentHealth <= 0.0f;
}

void UAttributeComponent::RemoveHealthBonus(float Delta)
{
	// Health bonus may be removed (Delta should be the same value that was added).
	if (GetOwnerRole() != ROLE_Authority) return;
	if (FMath::IsNearlyZero(Delta)) return;

	MaxHealth -= Delta;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
	bIsDead = CurrentHealth <= 0.0f;
}
