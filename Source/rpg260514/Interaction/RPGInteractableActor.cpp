// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/RPGInteractableActor.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

ARPGInteractableActor::ARPGInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	InteractionPrompt = FText::FromString(TEXT("Press E to interact"));
	UsedInteractionPrompt = FText::FromString(TEXT("Already used"));

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	InteractionBounds = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionBounds"));
	InteractionBounds->SetupAttachment(SceneRoot);
	InteractionBounds->SetSphereRadius(260.0f);
	InteractionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBounds->SetCollisionResponseToAllChannels(ECR_Overlap);
	InteractionBounds->SetGenerateOverlapEvents(true);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(SceneRoot);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshAsset.Object != nullptr)
	{
		VisualMesh->SetStaticMesh(CubeMeshAsset.Object);
		VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
		VisualMesh->SetRelativeScale3D(FVector(1.6f, 1.6f, 2.0f));
	}
}

bool ARPGInteractableActor::CanInteract_Implementation(AActor* Interactor)
{
	return bCanInteract;
}

FText ARPGInteractableActor::GetInteractionPrompt_Implementation(AActor* Interactor)
{
	return bCanInteract ? InteractionPrompt : UsedInteractionPrompt;
}

void ARPGInteractableActor::Interact_Implementation(AActor* Interactor)
{
	if (!bCanInteract)
	{
		return;
	}

	if (bLogDebugInteraction && GEngine != nullptr)
	{
		const FString DebugText = FString::Printf(TEXT("Interacted: %s"), *GetName());
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, DebugText);
	}

	OnInteracted(Interactor);

	if (bDisableAfterInteraction)
	{
		bCanInteract = false;
	}
}
