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

    switch (StationType)
    {
    case ELobbyStationType::Shop:
        Interactor->Client_OpenShop();
        break;
    case ELobbyStationType::LevelUp:
        Interactor->Client_OpenLevelUp();
        break;
    case ELobbyStationType::Equipment:
        Interactor->Client_OpenEquip();
        break;
    case ELobbyStationType::ReadyUp:
        Interactor->Server_SetReadyForNextRound(true);
        break;
    default:
        break;
    }
}

FText ALobbyStation::GetInteractionText_Implementation() const
{
    FString Action = (StationType == ELobbyStationType::ReadyUp) ? "Ready Up" : "Open";
    return FText::Format(NSLOCTEXT("CK", "LobbyInteract", "{0} {1}"), FText::FromString(Action), StationName);
}
