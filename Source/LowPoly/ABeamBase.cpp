#include "ABeamBase.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

ABeamBase::ABeamBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BeamComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BeamComponent"));
	BeamComponent->SetupAttachment(Root);

    ImpactComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ImpactComponent"));
    ImpactComponent->SetupAttachment(Root);
    ImpactComponent->bAutoActivate = false;

    bIsHitting = false;
}

void ABeamBase::InitBeamTracking(UStaticMeshComponent* StaffMesh, FName SocketName)
{
	TrackingMesh   = StaffMesh;
	TrackingSocket = SocketName;
}

void ABeamBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    const FVector CurrentOrigin = GetActorLocation();

	// Server: smooth track socket
	if (HasAuthority() && TrackingMesh.IsValid() && !BeamEndLocation.IsZero())
	{
		const FVector SocketPos = TrackingMesh->GetSocketLocation(TrackingSocket);
		ApplyBeamTransformAndParams(SocketPos, BeamEndLocation);
	}

	// All clients (including server): push Niagara & update looping audio locations
	if (!BeamEndLocation.IsZero())
	{
		if (BeamComponent)
		{
			BeamComponent->SetVectorParameter(StartPointParam, CurrentOrigin);
			BeamComponent->SetVectorParameter(EndPointParam,   BeamEndLocation);
		}

        if (FlightAudio && FlightAudio->IsPlaying())
        {
            FlightAudio->SetWorldLocation(CurrentOrigin);
        }

        if (ImpactAudio && ImpactAudio->IsPlaying())
        {
            ImpactAudio->SetWorldLocation(BeamEndLocation);
        }

        if (ImpactComponent && ImpactComponent->IsActive())
        {
            ImpactComponent->SetWorldLocation(BeamEndLocation);
            // Re-orient impact to face back towards the source
            const FVector DirToSource = (CurrentOrigin - BeamEndLocation).GetSafeNormal();
            if (!DirToSource.IsNearlyZero())
            {
                ImpactComponent->SetWorldRotation(UKismetMathLibrary::MakeRotFromX(DirToSource));
            }
        }
	}
}

void ABeamBase::UpdateBeam(FVector Start, FVector End, bool bHit, FHitResult Hit)
{
	const FVector EffectiveEnd = bHit ? Hit.ImpactPoint : End;

	BeamEndLocation = EffectiveEnd;
    bIsHitting = bHit;

	ApplyBeamTransformAndParams(Start, EffectiveEnd);
    UpdateDynamicEffects();
}

void ABeamBase::OnRep_BeamEndLocation()
{
	if (BeamComponent)
	{
		BeamComponent->SetVectorParameter(StartPointParam, GetActorLocation());
		BeamComponent->SetVectorParameter(EndPointParam,   BeamEndLocation);
	}
}

void ABeamBase::OnRep_bIsHitting()
{
    UpdateDynamicEffects();
}

void ABeamBase::ApplyBeamTransformAndParams(FVector WorldStart, FVector WorldEnd)
{
	SetActorLocation(WorldStart);

	const FVector Direction = (WorldEnd - WorldStart).GetSafeNormal();
	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(UKismetMathLibrary::MakeRotFromX(Direction));
	}

	if (BeamComponent)
	{
		BeamComponent->SetVectorParameter(StartPointParam, WorldStart);
		BeamComponent->SetVectorParameter(EndPointParam,   WorldEnd);
	}
}

void ABeamBase::UpdateDynamicEffects()
{
    // --- Flight Audio ---
    if (BeamFlightSound)
    {
        if (!FlightAudio)
        {
            FlightAudio = UGameplayStatics::SpawnSoundAttached(BeamFlightSound, Root, NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, false, 1.f, 1.f, 0.f, BeamSoundAttenuation);
            if (FlightAudio) FlightAudio->bAutoDestroy = false;
        }
        
        if (FlightAudio && !FlightAudio->IsPlaying())
        {
            FlightAudio->Play();
        }
    }

    // --- Impact Audio ---
    if (BeamImpactSound)
    {
        if (!ImpactAudio)
        {
            // Spawn at end location, not attached to root since root is at the staff
            ImpactAudio = UGameplayStatics::SpawnSoundAtLocation(this, BeamImpactSound, BeamEndLocation, FRotator::ZeroRotator, 1.f, 1.f, 0.f, BeamSoundAttenuation);
            if (ImpactAudio) ImpactAudio->bAutoDestroy = false;
        }

        if (ImpactAudio)
        {
            if (bIsHitting && !ImpactAudio->IsPlaying())
            {
                ImpactAudio->Play();
            }
            else if (!bIsHitting && ImpactAudio->IsPlaying())
            {
                ImpactAudio->FadeOut(SoundFadeOutDuration, 0.f);
            }
        }
    }

    // --- Impact VFX ---
    if (ImpactEffect)
    {
        if (!ImpactComponent->GetAsset())
        {
            ImpactComponent->SetAsset(ImpactEffect);
        }

        if (bIsHitting && !ImpactComponent->IsActive())
        {
            ImpactComponent->Activate();
        }
        else if (!bIsHitting && ImpactComponent->IsActive())
        {
            ImpactComponent->Deactivate();
        }
    }
}

void ABeamBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // When the beam actor is destroyed (channel ends), force-stop all audio immediately or with a quick fade.
    if (FlightAudio)
    {
        if (SoundFadeOutDuration > 0.0f) FlightAudio->FadeOut(SoundFadeOutDuration, 0.0f);
        else FlightAudio->Stop();
    }

    if (ImpactAudio)
    {
        if (SoundFadeOutDuration > 0.0f) ImpactAudio->FadeOut(SoundFadeOutDuration, 0.0f);
        else ImpactAudio->Stop();
    }

    Super::EndPlay(EndPlayReason);
}

void ABeamBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABeamBase, BeamEndLocation);
    DOREPLIFETIME(ABeamBase, bIsHitting);
}
