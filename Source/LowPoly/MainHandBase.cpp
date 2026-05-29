#include "MainHandBase.h"
#include "AttributeComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AMainHandBase::AMainHandBase()
{
	ItemType = EItemType::Melee;
    BaseDamage.Physical = 10.0f;
	AttackMontage = nullptr;
	ComboSectionNames.Add(FName("Attack_1"));
	ComboSectionNames.Add(FName("Attack_2"));
	ComboSectionNames.Add(FName("Attack_3"));
	MaxHitTargets = 1;
	bDrawDebugAttack = false;
	bHitWindowOpen = false;
	HitSound = nullptr;
	bUseMana = false;
	ManaCost = 0.0f;
    StaminaCost = 0.0f;

	HitboxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("HitboxComponent"));
	HitboxComponent->SetupAttachment(Mesh);
	HitboxComponent->SetRelativeLocation(FVector::ZeroVector);
	HitboxComponent->SetRelativeRotation(FRotator::ZeroRotator);
	HitboxComponent->SetBoxExtent(HitboxExtent);
	HitboxComponent->SetAbsolute(false, false, true);
	HitboxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitboxComponent->SetGenerateOverlapEvents(true);
	HitboxComponent->SetCollisionObjectType(ECC_WorldDynamic);
	HitboxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitboxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HitboxComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

void AMainHandBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

float AMainHandBase::GetManaCost() const
{
    float RarityMana = GetCurrentManaCost();
    return (RarityMana > 0.0f) ? RarityMana : ManaCost;
}

float AMainHandBase::GetStaminaCost() const
{
    float RarityStamina = GetCurrentStaminaCost();
    return (RarityStamina > 0.0f) ? RarityStamina : StaminaCost;
}


FDamageBundle AMainHandBase::GetScaledDamage(const UAttributeComponent* Attributes) const
{
	// Get base damage for current rarity (uses rarity stats if defined, otherwise base damage with rarity scale)
	FDamageBundle Scaled = GetCurrentDamage();
	Scaled.Scale(ComputeAttributeMultiplier(Attributes));
	return Scaled;
}

FName AMainHandBase::GetComboSectionName(int32 ComboIndex) const
{
	return ComboSectionNames.IsValidIndex(ComboIndex) ? ComboSectionNames[ComboIndex] : NAME_None;
}

bool AMainHandBase::ApplyMeleeHit(AActor* AttackingActor)
{
	if (!HasAuthority() || !HitboxComponent) return false;

    APlayerCharacter* AttackingCharacter = Cast<APlayerCharacter>(AttackingActor);
    if (!AttackingCharacter) return false;

    FVector BoxLocation = HitboxComponent->GetComponentLocation();
    FQuat BoxRotation = HitboxComponent->GetComponentQuat();
    FVector BoxExtent = HitboxComponent->GetScaledBoxExtent();

    if (bDrawDebugAttack)
    {
        DrawDebugBox(GetWorld(), BoxLocation, BoxExtent, BoxRotation, FColor::Red, false, 0.5f);
    }

	UAttributeComponent* AttackerAttributes = AttackingCharacter->GetAttributeComponent();
	if (!AttackerAttributes) return false;

    const FDamageBundle FinalDamage = GetScaledDamage(AttackerAttributes);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

    TArray<AActor*> ActorsToIgnore = { this, AttackingCharacter };

	TArray<AActor*> OverlappingActors;
    UKismetSystemLibrary::BoxOverlapActors(
        GetWorld(),
        BoxLocation,
        BoxExtent,
        ObjectTypes,
        nullptr,
        ActorsToIgnore,
        OverlappingActors
    );

	int32 HitTargets = 0;
	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor || HitActorsThisWindow.Contains(Actor)) continue;

		UAttributeComponent* TargetAttributes = Actor->FindComponentByClass<UAttributeComponent>();
		if (!TargetAttributes || TargetAttributes->IsDead()) continue;

		if (TargetAttributes->ApplyCombatDamage(FinalDamage, AttackingCharacter))
		{
            UE_LOG(LogTemp, Log, TEXT("Melee Hit Registered on: %s"), *Actor->GetName());
			HitActorsThisWindow.Add(Actor);
			HitTargets++;
			Multicast_OnMeleeHit();
		}

		if (HitTargets >= MaxHitTargets) break;
	}

	return true;
}

void AMainHandBase::BeginMeleeHitWindow(AActor* AttackingCharacter)
{
    if (HitboxComponent) HitboxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
	if (!HasAuthority() || !AttackingCharacter)
	{
		return;
	}

	bHitWindowOpen = true;
	HitActorsThisWindow.Empty();
	ApplyMeleeHit(AttackingCharacter);
}

void AMainHandBase::TickMeleeHitWindow(AActor* AttackingCharacter)
{
	if (!HasAuthority() || !bHitWindowOpen || !AttackingCharacter)
	{
		return;
	}

	ApplyMeleeHit(AttackingCharacter);
}

void AMainHandBase::EndMeleeHitWindow()
{
    if (HitboxComponent) HitboxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
	if (!HasAuthority())
	{
		return;
	}

	bHitWindowOpen = false;
	HitActorsThisWindow.Empty();
}

float AMainHandBase::ComputeAttributeMultiplier(const UAttributeComponent* Attributes) const
{
	if (!Attributes) return 1.0f;

	const float RarityScale = AItemBase::GetRarityStrengthScale(Rarity);

	const bool bUseLetterScaling =
		StrengthScalingLetter != EStatLetterScaling::Unspecified ||
		DexterityScalingLetter != EStatLetterScaling::Unspecified ||
		MagicScalingLetter != EStatLetterScaling::Unspecified ||
		LuckScalingLetter != EStatLetterScaling::Unspecified;

	const float StrengthCoeff  = bUseLetterScaling ? AItemBase::GetLetterStatScalingCoeff(StrengthScalingLetter)  : StrengthScaling;
	const float DexterityCoeff = bUseLetterScaling ? AItemBase::GetLetterStatScalingCoeff(DexterityScalingLetter) : DexterityScaling;
	const float MagicCoeff     = bUseLetterScaling ? AItemBase::GetLetterStatScalingCoeff(MagicScalingLetter)     : MagicScaling;
	const float LuckCoeff      = bUseLetterScaling ? AItemBase::GetLetterStatScalingCoeff(LuckScalingLetter)      : LuckScaling;

	const float WeightedSum =
		  Attributes->Strength  * StrengthCoeff
		+ Attributes->Dexterity * DexterityCoeff
		+ Attributes->Magic     * MagicCoeff
		+ Attributes->Luck      * LuckCoeff;

	return 1.0f + RarityScale * WeightedSum;
}

bool AMainHandBase::IsRanged() const
{
	return false;
}

bool AMainHandBase::IsMagic() const
{
	return false;
}

bool AMainHandBase::IsTwoHanded() const
{
	return bTwoHanded;
}

FSkillSlotInfo AMainHandBase::GetLMBSkillInfo() const
{
	FSkillSlotInfo Info;
	Info.bIsValid = true;
	Info.Icon = PrimaryActionIcon;
	Info.DisplayName = PrimaryActionName;
	Info.bHasCooldown = false;
	Info.CooldownProgress = 1.0f;
	return Info;
}

FSkillSlotInfo AMainHandBase::GetRMBSkillInfo() const
{
	return FSkillSlotInfo();
}

EWeaponHoldStance AMainHandBase::GetHoldStance_Implementation() const
{
	return bTwoHanded ? EWeaponHoldStance::TwoHandedMelee : EWeaponHoldStance::OneHandedMelee;
}

void AMainHandBase::Multicast_OnMeleeHit_Implementation()
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}
	BP_OnMeleeHit();
}
