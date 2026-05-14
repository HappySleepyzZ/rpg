// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/RPGPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"

ARPGPlayerController::ARPGPlayerController()
{
	bShowMouseCursor = false;

	// 探索输入上下文默认指向项目资产；蓝图子类仍可覆盖为测试或平台专用上下文。
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> ExplorationMappingAsset(TEXT("/Game/Input/IMC_Exploration.IMC_Exploration"));
	ExplorationMappingContext = ExplorationMappingAsset.Object;
}

void ARPGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (InputSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RPGPlayerController 无法获取 Enhanced Input LocalPlayerSubsystem。"));
		return;
	}

	if (ExplorationMappingContext != nullptr)
	{
		InputSubsystem->AddMappingContext(ExplorationMappingContext, ExplorationMappingPriority);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("尚未设置 ExplorationMappingContext。请在玩家控制器蓝图中指定 IMC_Exploration。"));
	}
}
