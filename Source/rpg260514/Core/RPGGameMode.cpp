// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/RPGGameMode.h"

#include "Character/RPGPlayerCharacter.h"
#include "Core/RPGPlayerController.h"

ARPGGameMode::ARPGGameMode()
{
	DefaultPawnClass = ARPGPlayerCharacter::StaticClass();
	PlayerControllerClass = ARPGPlayerController::StaticClass();
}
