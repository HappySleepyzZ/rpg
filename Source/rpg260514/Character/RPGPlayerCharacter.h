// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RPGPlayerCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UAnimSequenceBase;
class USpringArmComponent;
struct FInputActionValue;

UCLASS()
class RPG260514_API ARPGPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARPGPlayerCharacter();

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Zoom(const FInputActionValue& Value);
	void StartDash();
	void StopDash();
	void StopDashMovement();
	void Interact();
	void PrimaryAction();
	bool IsLikelyRootMotionDashAnimation(const UAnimSequenceBase* Animation) const;
	void UpdateMovementDebugText() const;
	FString GetMovementModeDebugText() const;
	FString GetDashStateDebugText() const;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true", DisplayName = "Dash Action"))
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DashStrength = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float DashDuration = 0.16f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DashCooldown = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	bool bDashUseInputDirection = true;

	// 勾选后优先播放 RootMotion 闪避动画。位移仍会叠加一个短促冲量，保证动画/Slot 出问题时玩家也能看到闪避发生。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	bool bDashUseRootMotionAnimation = false;

	// 原型调试阶段保持开启：用代码给一次明确位移，避免只依赖 RootMotion 动画导致“按了但没变化”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	bool bDashApplyMovementImpulse = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimSequenceBase> DashAnimation;

	// 当前闪避位移由 LaunchCharacter 控制。若 DashAnimation 误填 RootMotion 动画，运行时会改播这个兜底动画，避免 Mesh 离开胶囊后回弹。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimSequenceBase> DashFallbackAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DashAnimationBlendIn = 0.04f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DashAnimationBlendOut = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float DashAnimationPlayRate = 1.2f;

	// 动态 Montage 播放使用的 Slot。官方 ABP_Unarmed 默认支持 DefaultSlot，后续自建 ABP 时可在蓝图里改成专用 Slot。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	FName DashAnimationSlotName = TEXT("DefaultSlot");

	bool bCanDash = true;
	bool bDashMovementActive = false;
	float LastDashStartTime = -1.0f;
	FVector LastDashDirection = FVector::ZeroVector;
	FString LastDashEvent = TEXT("Never");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	bool bShowMovementDebug = true;

	// 第三人称镜头参数先集中在角色上，方便原型阶段在 BP_PlayerCharacter 中调试。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Tuning", meta = (AllowPrivateAccess = "true"))
	float CameraDistance = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Tuning", meta = (AllowPrivateAccess = "true"))
	float MinCameraDistance = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Tuning", meta = (AllowPrivateAccess = "true"))
	float MaxCameraDistance = 700.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Tuning", meta = (AllowPrivateAccess = "true"))
	float ZoomStep = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Tuning", meta = (AllowPrivateAccess = "true"))
	float MouseLookSensitivity = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Tuning", meta = (AllowPrivateAccess = "true"))
	float GamepadLookSensitivity = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
