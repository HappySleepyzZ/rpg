// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/InteractionDetectorComponent.h"

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
		if (IsValidInteractable(OverlappingActor))
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
		if (!IsValidInteractable(Candidate))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(OwnerActor->GetActorLocation(), Candidate->GetActorLocation());
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
	if (IsValidInteractable(OtherActor))
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

bool UInteractionDetectorComponent::IsValidInteractable(AActor* Candidate) const
{
	AActor* OwnerActor = GetOwner();
	if (Candidate == nullptr || Candidate == OwnerActor || !Candidate->Implements<UInteractableInterface>())
	{
		return false;
	}

	return IInteractableInterface::Execute_CanInteract(Candidate, OwnerActor);
}

void UInteractionDetectorComponent::RemoveInvalidCandidates()
{
	CandidateActors.RemoveAll([this](const TWeakObjectPtr<AActor>& CandidatePtr)
	{
		return !IsValidInteractable(CandidatePtr.Get());
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
