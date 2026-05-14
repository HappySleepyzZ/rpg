// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RPGPlayerAnimInstance.generated.h"

UCLASS()
class RPG260514_API URPGPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 水平移动速度，动画蓝图用它在 Idle / Walk / Run 之间切换。
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed = 0.0f;

	// 是否处于下落状态。之后接跳跃、坠落、落地动画时使用。
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling = false;
};
