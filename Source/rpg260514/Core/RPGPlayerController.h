// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RPGPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class RPG260514_API ARPGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARPGPlayerController();

protected:
	virtual void BeginPlay() override;

private:
	// 探索输入上下文：移动、视角、跳跃、交互、基础行动。
	// 后续进入战斗时可以叠加 CombatMappingContext，而不是改动角色输入代码。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> ExplorationMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	int32 ExplorationMappingPriority = 0;
};
