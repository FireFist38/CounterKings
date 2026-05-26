#include "LevelUpWidget.h"
#include "CKXPBarWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "CKGameMode.h"
#include "CKGameState.h"
#include "Kismet/GameplayStatics.h"

void ULevelUpWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Health_Plus) Health_Plus->OnClicked.AddDynamic(this, &ULevelUpWidget::AddHealth);
	if (Health_Minus) Health_Minus->OnClicked.AddDynamic(this, &ULevelUpWidget::SubHealth);
	if (Strength_Plus) Strength_Plus->OnClicked.AddDynamic(this, &ULevelUpWidget::AddStrength);
	if (Strength_Minus) Strength_Minus->OnClicked.AddDynamic(this, &ULevelUpWidget::SubStrength);
	if (Dexterity_Plus) Dexterity_Plus->OnClicked.AddDynamic(this, &ULevelUpWidget::AddDexterity);
	if (Dexterity_Minus) Dexterity_Minus->OnClicked.AddDynamic(this, &ULevelUpWidget::SubDexterity);
	if (Magic_Plus) Magic_Plus->OnClicked.AddDynamic(this, &ULevelUpWidget::AddMagic);
	if (Magic_Minus) Magic_Minus->OnClicked.AddDynamic(this, &ULevelUpWidget::SubMagic);
	if (Luck_Plus) Luck_Plus->OnClicked.AddDynamic(this, &ULevelUpWidget::AddLuck);
	if (Luck_Minus) Luck_Minus->OnClicked.AddDynamic(this, &ULevelUpWidget::SubLuck);
	if (ConfirmButton) ConfirmButton->OnClicked.AddDynamic(this, &ULevelUpWidget::ConfirmUpgrades);
	if (BuyXPButton) BuyXPButton->OnClicked.AddDynamic(this, &ULevelUpWidget::OnBuyXPClicked);

	UpdateUI();
}

void ULevelUpWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (GS && TimerText) TimerText->SetText(FText::AsNumber(FMath::CeilToInt(GS->GetRemainingTime())));
}

void ULevelUpWidget::AddHealth() { ModifyPending(EAttributeType::Health, 1); }
void ULevelUpWidget::SubHealth() { ModifyPending(EAttributeType::Health, -1); }
void ULevelUpWidget::AddStrength() { ModifyPending(EAttributeType::Strength, 1); }
void ULevelUpWidget::SubStrength() { ModifyPending(EAttributeType::Strength, -1); }
void ULevelUpWidget::AddDexterity() { ModifyPending(EAttributeType::Dexterity, 1); }
void ULevelUpWidget::SubDexterity() { ModifyPending(EAttributeType::Dexterity, -1); }
void ULevelUpWidget::AddMagic() { ModifyPending(EAttributeType::Magic, 1); }
void ULevelUpWidget::SubMagic() { ModifyPending(EAttributeType::Magic, -1); }
void ULevelUpWidget::AddLuck() { ModifyPending(EAttributeType::Luck, 1); }
void ULevelUpWidget::SubLuck() { ModifyPending(EAttributeType::Luck, -1); }

void ULevelUpWidget::ModifyPending(EAttributeType Attribute, int32 Amount)
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (!Character || !Character->GetAttributeComponent()) return;

	int32 Available = Character->GetAttributeComponent()->GetAttributePoints();
    
    int32* PendingPtr = nullptr;
    int32* CommittedPtr = nullptr;

    switch(Attribute)
    {
        case EAttributeType::Health: PendingPtr = &PendingHealth; CommittedPtr = &CommittedHealth; break;
        case EAttributeType::Strength: PendingPtr = &PendingStrength; CommittedPtr = &CommittedStrength; break;
        case EAttributeType::Dexterity: PendingPtr = &PendingDexterity; CommittedPtr = &CommittedDexterity; break;
        case EAttributeType::Magic: PendingPtr = &PendingMagic; CommittedPtr = &CommittedMagic; break;
        case EAttributeType::Luck: PendingPtr = &PendingLuck; CommittedPtr = &CommittedLuck; break;
    }

    int32 TotalPending = PendingHealth + PendingStrength + PendingDexterity + PendingMagic + PendingLuck;

	if (Amount > 0)
	{
		if (TotalPending < Available)
		{
			(*PendingPtr)++;
		}
	}
	else if (Amount < 0)
	{
		if ((*PendingPtr) > (*CommittedPtr))
		{
			(*PendingPtr)--;
		}
	}
	UpdateUI();
}

void ULevelUpWidget::UpdateUI()
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (!Character || !Character->GetAttributeComponent()) return;

	int32 Available = Character->GetAttributeComponent()->GetAttributePoints();
    int32 TotalPending = PendingHealth + PendingStrength + PendingDexterity + PendingMagic + PendingLuck;

	PointsRemainingText->SetText(FText::AsNumber(FMath::Max(0, Available - TotalPending)));
	if (CurrentLevelText) 
    {
        CurrentLevelText->SetText(FText::AsNumber(Character->GetAttributeComponent()->GetLevel()));
    }

    if (XPBar)
    {
        XPBar->UpdateXPBar(Character->GetAttributeComponent()->GetLevel(), 
                           Character->GetAttributeComponent()->GetCurrentXP(), 
                           Character->GetAttributeComponent()->GetXPThreshold());
    }

	Health_PendingText->SetText(FText::AsNumber(PendingHealth));
	Strength_PendingText->SetText(FText::AsNumber(PendingStrength));
	Dexterity_PendingText->SetText(FText::AsNumber(PendingDexterity));
	Magic_PendingText->SetText(FText::AsNumber(PendingMagic));
	Luck_PendingText->SetText(FText::AsNumber(PendingLuck));

	Health_Plus->SetIsEnabled(TotalPending < Available);
	Health_Minus->SetIsEnabled(PendingHealth > CommittedHealth);
	Strength_Plus->SetIsEnabled(TotalPending < Available);
	Strength_Minus->SetIsEnabled(PendingStrength > CommittedStrength);
	Dexterity_Plus->SetIsEnabled(TotalPending < Available);
	Dexterity_Minus->SetIsEnabled(PendingDexterity > CommittedDexterity);
	Magic_Plus->SetIsEnabled(TotalPending < Available);
	Magic_Minus->SetIsEnabled(PendingMagic > CommittedMagic);
	Luck_Plus->SetIsEnabled(TotalPending < Available);
	Luck_Minus->SetIsEnabled(PendingLuck > CommittedLuck);
	ConfirmButton->SetIsEnabled(TotalPending > (CommittedHealth + CommittedStrength + CommittedDexterity + CommittedMagic + CommittedLuck));

    if (BuyXPButton)
    {
        BuyXPButton->SetIsEnabled(Character->GetAttributeComponent()->GetGold() >= 100);
    }
}

void ULevelUpWidget::ConfirmUpgrades()
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (!Character || !Character->GetAttributeComponent()) return;

	Character->GetAttributeComponent()->Server_CommitAttributeUpgrades(
		PendingHealth - CommittedHealth,
		PendingStrength - CommittedStrength,
		PendingDexterity - CommittedDexterity,
		PendingMagic - CommittedMagic,
		PendingLuck - CommittedLuck);

    CommittedHealth = PendingHealth;
    CommittedStrength = PendingStrength;
    CommittedDexterity = PendingDexterity;
    CommittedMagic = PendingMagic;
    CommittedLuck = PendingLuck;

	UpdateUI();
}

void ULevelUpWidget::OnBuyXPClicked()
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (Character && Character->GetAttributeComponent())
	{
		Character->GetAttributeComponent()->Server_PurchaseXP();
		
        FTimerHandle DelayTimer;
        GetWorld()->GetTimerManager().SetTimer(DelayTimer, this, &ULevelUpWidget::UpdateUI, 0.25f, false);
	}
}
