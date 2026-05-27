#include "InteractionComponent.h"
#include "Components/SphereComponent.h"
#include "CKInteractable.h"
#include "PlayerCharacter.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	InteractionRadius = 200.0f;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Attach sphere to owner's root
	AActor* Owner = GetOwner();
	if (Owner)
	{
		InteractionSphere = NewObject<USphereComponent>(Owner, TEXT("InteractionSphere"));
		InteractionSphere->RegisterComponent();
		InteractionSphere->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		InteractionSphere->SetSphereRadius(InteractionRadius);
		InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		InteractionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	}
}

void UInteractionComponent::PerformInteraction()
{
	Server_PerformInteraction();
}

void UInteractionComponent::Server_PerformInteraction_Implementation()
{
	AActor* BestInteractable = GetBestInteractable();
	if (BestInteractable)
	{
		APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwner());
		if (Character)
		{
            ICKInteractable::Execute_Interact(BestInteractable, Character);
		}
	}
}

AActor* UInteractionComponent::GetBestInteractable()
{
	if (!InteractionSphere) return nullptr;

	TArray<AActor*> OverlappingActors;
	InteractionSphere->GetOverlappingActors(OverlappingActors);

	AActor* ClosestInteractable = nullptr;
	float MinDistanceSq = FMath::Square(InteractionRadius + 100.0f);

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->GetClass()->ImplementsInterface(UCKInteractable::StaticClass()))
		{
			float DistanceSq = FVector::DistSquared(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
			if (DistanceSq < MinDistanceSq)
			{
				MinDistanceSq = DistanceSq;
				ClosestInteractable = Actor;
			}
		}
	}

	return ClosestInteractable;
}
