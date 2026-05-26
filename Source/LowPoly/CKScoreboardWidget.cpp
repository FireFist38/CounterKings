#include "CKScoreboardWidget.h"
#include "CKGameState.h"
#include "CKPlayerState.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"

void UCKScoreboardWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ACKGameState* GS = Cast<ACKGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (GS)
	{
		if (RoundText) RoundText->SetText(FText::AsNumber(GS->GetCurrentRound()));
		if (TimerText) TimerText->SetText(FText::AsNumber(FMath::CeilToInt(GS->GetRemainingTime())));
	}

    UpdateScoreboard();
}

void UCKScoreboardWidget::UpdateScoreboard()
{
    TArray<APlayerState*> PlayerStates = GetWorld()->GetGameState()->PlayerArray;
    
    struct FSlot { UTextBlock* Name; UProgressBar* Bar; UTextBlock* Val; UTextBlock* Gold; };
    FSlot Slots[] = { 
        {Name0, HPBar0, HPVal0, Gold0}, {Name1, HPBar1, HPVal1, Gold1},
        {Name2, HPBar2, HPVal2, Gold2}, {Name3, HPBar3, HPVal3, Gold3},
        {Name4, HPBar4, HPVal4, Gold4}, {Name5, HPBar5, HPVal5, Gold5},
        {Name6, HPBar6, HPVal6, Gold6}, {Name7, HPBar7, HPVal7, Gold7},
        {Name8, HPBar8, HPVal8, Gold8}, {Name9, HPBar9, HPVal9, Gold9}
    };

    for (int32 i = 0; i < 10; i++)
    {
        if (PlayerStates.IsValidIndex(i))
        {
            APlayerState* PS = PlayerStates[i];
            ACKPlayerState* CKPS = Cast<ACKPlayerState>(PS);
            APlayerCharacter* Char = Cast<APlayerCharacter>(PS->GetPawn());
            
            if (Slots[i].Name) Slots[i].Name->SetText(FText::FromString(PS->GetPlayerName()));
            
            if (CKPS)
            {
                if (Slots[i].Bar) Slots[i].Bar->SetPercent(CKPS->MatchHealth / 100.0f);
                if (Slots[i].Val) Slots[i].Val->SetText(FText::AsNumber(CKPS->MatchHealth));
            }

            if (Char && Char->GetAttributeComponent() && Slots[i].Gold)
                Slots[i].Gold->SetText(FText::AsNumber(Char->GetAttributeComponent()->GetGold()));
        }
        else
        {
            if (Slots[i].Name) Slots[i].Name->SetText(FText::GetEmpty());
            if (Slots[i].Bar) Slots[i].Bar->SetPercent(0.0f);
            if (Slots[i].Val) Slots[i].Val->SetText(FText::GetEmpty());
            if (Slots[i].Gold) Slots[i].Gold->SetText(FText::GetEmpty());
        }
    }
}
