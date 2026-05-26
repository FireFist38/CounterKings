#include "TestDummy.h"
#include "AttributeComponent.h"
#include "TestDummyHealthBar.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "ItemBase.h"

ATestDummy::ATestDummy()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetCapsuleHalfHeight(88.f);
	Capsule->SetCapsuleRadius(34.f);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Capsule->SetCollisionObjectType(ECC_WorldDynamic);
	Capsule->SetCollisionResponseToAllChannels(ECR_Block);
	SetRootComponent(Capsule);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Capsule);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(Capsule);
	HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::World);
	HealthBarWidget->SetDrawSize(FVector2D(200.f, 25.f));
}

void ATestDummy::BeginPlay()
{
	Super::BeginPlay();

	if (HealthBarWidgetClass)
	{
		HealthBarWidget->SetWidgetClass(HealthBarWidgetClass);
		HealthBarWidget->InitWidget();

		UTestDummyHealthBar* HealthBarInstance = Cast<UTestDummyHealthBar>(HealthBarWidget->GetWidget());
		if (HealthBarInstance)
		{
			HealthBarInstance->OwningDummy = this;
		}
	}

	if (HasAuthority() && AttackInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			AttackTimerHandle,
			this,
			&ATestDummy::PerformAttackSweep,
			AttackInterval,
			true
		);
	}
}

void ATestDummy::PerformAttackSweep()
{
	// Flash a debug sphere so the swing is visible
	DrawDebugSphere(GetWorld(), GetActorLocation(), AttackRange, 16, FColor::Orange, false, AttackInterval * 0.4f);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore = { this };
	TArray<AActor*> HitActors;

	UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		AttackRange,
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		HitActors
	);

	for (AActor* Actor : HitActors)
	{
		UAttributeComponent* TargetAttr = Actor->FindComponentByClass<UAttributeComponent>();
		if (TargetAttr && !TargetAttr->IsDead())
		{
			TargetAttr->ApplyCombatDamage(DamageBundle, this);
		}
	}
}
