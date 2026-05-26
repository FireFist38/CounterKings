#include "SpectralShieldBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PlayerCharacter.h"
#include "AttributeComponent.h"
#include "ItemBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

ASpectralShieldBase::ASpectralShieldBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(SceneRoot);
	
	// Block everything (players, projectiles, melee hitbox traces)
	CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetGenerateOverlapEvents(true);

	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	ShieldMesh->SetupAttachment(CollisionBox);
	ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Visuals only

	ShieldHitSound = nullptr;
	ShieldHitEffect = nullptr;
	ShieldHitEffectScale = 1.0f;
}

void ASpectralShieldBase::BeginPlay()
{
	Super::BeginPlay();
}

void ASpectralShieldBase::ConfigureShield(const FVector& BoxExtent)
{
	if (CollisionBox)
	{
		CollisionBox->SetBoxExtent(BoxExtent);
	}
}

void ASpectralShieldBase::SetCaster(AActor* Caster)
{
	CasterActor = Caster;
	if (CollisionBox && Caster)
	{
		CollisionBox->IgnoreActorWhenMoving(Caster, true);
	}
}

bool ASpectralShieldBase::AbsorbDamage(AActor* DamageInstigator, const FDamageBundle& Damage)
{
	if (!CasterActor) return false;

	APlayerCharacter* PC = Cast<APlayerCharacter>(CasterActor);
	if (!PC) return false;

	UAttributeComponent* Attr = PC->GetAttributeComponent();
	if (!Attr) return false;

	const float TotalDamage = Damage.GetTotal();
	if (TotalDamage <= 0.0f) return true;

	// Try to consume mana — if it fails, the damage leaks through
	if (!Attr->ConsumeMana(TotalDamage))
	{
		return false;
	}

	// Play hit feedback on all clients
	Multicast_OnShieldHit();
	return true;
}

void ASpectralShieldBase::Multicast_OnShieldHit_Implementation()
{
	// Play impact sound at shield location
	if (ShieldHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShieldHitSound, GetActorLocation());
	}

	// Spawn impact VFX at shield location
	if (ShieldHitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ShieldHitEffect,
			GetActorLocation(),
			GetActorRotation(),
			FVector(ShieldHitEffectScale),
			true,
			true,
			ENCPoolMethod::None,
			true
		);
	}
}