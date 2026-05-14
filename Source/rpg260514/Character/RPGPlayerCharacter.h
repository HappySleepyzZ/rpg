// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RPGPlayerCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UBlendSpace;
class USpringArmComponent;
struct FInputActionValue;

UCLASS()
class RPG260514_API ARPGPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARPGPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();
	void PrimaryAction();
	void UpdatePrototypeLocomotionAnimation();

	// Enhanced Input 的输入资产由蓝图或编辑器资产指定，代码只依赖语义，不直接写死按键。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PrimaryActionInput;

	// 原型阶段的最小移动动画：直接用官方免费 BlendSpace 按速度驱动 Idle/Walk/Run。
	// 后续制作正式动画蓝图后，可以移除这个临时驱动。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Prototype", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBlendSpace> PrototypeLocomotionBlendSpace;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
