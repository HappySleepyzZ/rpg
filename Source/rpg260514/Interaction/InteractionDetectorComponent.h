// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "InteractionDetectorComponent.generated.h"

UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class RPG260514_API UInteractionDetectorComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UInteractionDetectorComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool TryInteract();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* GetBestInteractable() const;

protected:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	bool IsValidInteractable(AActor* Candidate) const;
	void RemoveInvalidCandidates();
	void ShowCurrentPrompt() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float InteractionRadius = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	bool bShowDebugPrompt = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	FText DefaultPrompt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	int32 PromptMessageKey = 260515;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> CandidateActors;
};
