#include "LobbyStation.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "PlayerCharacter.h"

ALobbyStation::ALobbyStation()
{
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Root);
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(Root);
    InteractionSphere->SetSphereRadius(200.0f);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
    PromptWidget->SetupAttachment(Root);
    PromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
    PromptWidget->SetDrawAtDesiredSize(true);

    StationType = ELobbyStationType::Shop;
    StationName = FText::FromString("Shop");
}

void ALobbyStation::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyStation::Interact_Implementation(APlayerCharacter* Interactor)
{
    if (!HasAuthority() || !Interactor) return;

    // Special logic for ReadyUp station
    if (StationType == ELobbyStationType::ReadyUp)
    {
        Interactor->Server_SetReadyForNextRound(true);
    }
    else
    {
        // Trigger Client RPC to open the specific UI tab
        Interactor->Client_OpenLobbyUI(StationType);
    }
}

FText ALobbyStation::GetInteractionText_Implementation() const
{
    FString Action = (StationType == ELobbyStationType::ReadyUp) ? "Ready Up" : "Open";
    return FText::Format(NSLOCTEXT("CK", "LobbyInteract", "{0} {1}"), FText::FromString(Action), StationName);
}
