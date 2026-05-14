// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/RPGPlayerAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void URPGPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ACharacter* Character = Cast<ACharacter>(TryGetPawnOwner());
	if (Character == nullptr)
	{
		GroundSpeed = 0.0f;
		bIsFalling = false;
		return;
	}

	const FVector Velocity = Character->GetVelocity();
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Length();

	const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	bIsFalling = Movement != nullptr && Movement->IsFalling();
}
