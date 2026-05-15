// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/InteractionDetectorComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Interaction/InteractableInterface.h"

UInteractionDetectorComponent::UInteractionDetectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	DefaultPrompt = FText::FromString(TEXT("Press E to interact"));

	InitSphereRadius(InteractionRadius);
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Overlap);
	SetGenerateOverlapEvents(true);
}

void UInteractionDetectorComponent::BeginPlay()
{
	Super::BeginPlay();

	SetSphereRadius(InteractionRadius);

	OnComponentBeginOverlap.AddDynamic(this, &UInteractionDetectorComponent::HandleBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UInteractionDetectorComponent::HandleEndOverlap);

	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);
	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (IsInteractableActor(OverlappingActor))
		{
			CandidateActors.AddUnique(TWeakObjectPtr<AActor>(OverlappingActor));
		}
	}
}

void UInteractionDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RemoveInvalidCandidates();
	ShowCurrentPrompt();
}

bool UInteractionDetectorComponent::TryInteract()
{
	AActor* Interactable = GetBestInteractable();
	AActor* Interactor = GetOwner();
	if (Interactable == nullptr || Interactor == nullptr)
	{
		return false;
	}

	IInteractableInterface::Execute_Interact(Interactable, Interactor);
	return true;
}

AActor* UInteractionDetectorComponent::GetBestInteractable() const
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		return nullptr;
	}

	AActor* BestActor = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (const TWeakObjectPtr<AActor>& CandidatePtr : CandidateActors)
	{
		AActor* Candidate = CandidatePtr.Get();
		if (!CanInteractWithActor(Candidate))
		{
			continue;
		}

		const float DistanceSquared = GetInteractionDistanceSquared(Candidate);
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestActor = Candidate;
		}
	}

	return BestActor;
}

void UInteractionDetectorComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsInteractableActor(OtherActor))
	{
		CandidateActors.AddUnique(TWeakObjectPtr<AActor>(OtherActor));
	}
}

void UInteractionDetectorComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CandidateActors.RemoveAll([OtherActor](const TWeakObjectPtr<AActor>& CandidatePtr)
	{
		return CandidatePtr.Get() == OtherActor;
	});
}

bool UInteractionDetectorComponent::IsInteractableActor(const AActor* Candidate) const
{
	return Candidate != nullptr && Candidate != GetOwner() && Candidate->Implements<UInteractableInterface>();
}

bool UInteractionDetectorComponent::CanInteractWithActor(AActor* Candidate) const
{
	AActor* OwnerActor = GetOwner();
	if (!IsInteractableActor(Candidate) || OwnerActor == nullptr)
	{
		return false;
	}
	return IInteractableInterface::Execute_CanInteract(Candidate, OwnerActor);
}

float UInteractionDetectorComponent::GetInteractionDistanceSquared(const AActor* Candidate) const
{
	const AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr || Candidate == nullptr)
	{
		return TNumericLimits<float>::Max();
	}

	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	FBox CandidateBounds(ForceInit);
	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
	Candidate->GetComponents(PrimitiveComponents);

	for (const UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent == nullptr || !PrimitiveComponent->IsRegistered() || !PrimitiveComponent->IsVisible())
		{
			continue;
		}

		if (PrimitiveComponent->GetCollisionEnabled() == ECollisionEnabled::QueryOnly)
		{
			continue;
		}

		CandidateBounds += PrimitiveComponent->Bounds.GetBox();
	}

	if (CandidateBounds.IsValid)
	{
		return FVector::DistSquared(OwnerLocation, CandidateBounds.GetClosestPointTo(OwnerLocation));
	}

	return FVector::DistSquared(OwnerLocation, Candidate->GetActorLocation());
}

void UInteractionDetectorComponent::RemoveInvalidCandidates()
{
	CandidateActors.RemoveAll([this](const TWeakObjectPtr<AActor>& CandidatePtr)
	{
		return !IsInteractableActor(CandidatePtr.Get());
	});
}

void UInteractionDetectorComponent::ShowCurrentPrompt() const
{
	if (!bShowDebugPrompt || GEngine == nullptr)
	{
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn != nullptr && !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	AActor* Interactable = GetBestInteractable();
	if (Interactable == nullptr)
	{
		return;
	}

	AActor* Interactor = GetOwner();
	FText Prompt = IInteractableInterface::Execute_GetInteractionPrompt(Interactable, Interactor);
	if (Prompt.IsEmpty())
	{
		Prompt = DefaultPrompt;
	}

	GEngine->AddOnScreenDebugMessage(PromptMessageKey, 0.0f, FColor::Yellow, Prompt.ToString(), false);
}
