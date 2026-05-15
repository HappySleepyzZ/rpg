// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/RPGPlayerCharacter.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ARPGPlayerCharacter::ARPGPlayerCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	Movement->JumpZVelocity = 500.0f;
	Movement->AirControl = 0.35f;
	Movement->MaxWalkSpeed = WalkSpeed;
	Movement->MinAnalogWalkSpeed = 20.0f;
	Movement->BrakingDecelerationWalking = 2000.0f;
	Movement->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = CameraDistance;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 默认使用项目内的 Enhanced Input 资产。字段仍然暴露给蓝图，后续可以按角色类型覆盖。
	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionAsset(TEXT("/Game/Input/Actions/IA_Move.IA_Move"));
	MoveAction = MoveActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionAsset(TEXT("/Game/Input/Actions/IA_Look.IA_Look"));
	LookAction = LookActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> JumpActionAsset(TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"));
	JumpAction = JumpActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> InteractActionAsset(TEXT("/Game/Input/Actions/IA_Interact.IA_Interact"));
	InteractAction = InteractActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> PrimaryActionAsset(TEXT("/Game/Input/Actions/IA_PrimaryAction.IA_PrimaryAction"));
	PrimaryActionInput = PrimaryActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> SprintActionAsset(TEXT("/Game/Input/Actions/IA_Sprint.IA_Sprint"));
	SprintAction = SprintActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> ZoomActionAsset(TEXT("/Game/Input/Actions/IA_Zoom.IA_Zoom"));
	ZoomAction = ZoomActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> DashAnimationAsset(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Dash.MM_Dash"));
	DashAnimation = DashAnimationAsset.Object;
	DashFallbackAnimation = DashAnimationAsset.Object;
}

void ARPGPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RPGPlayerCharacter 需要 Enhanced Input Component。请确认项目没有回退到旧输入系统。"));
		return;
	}

	// 这里不再使用 BindAxis / BindAction 字符串，避免新旧输入系统混用。
	// 输入资产为空时只跳过对应绑定，方便先编译 C++，再在编辑器中补 IA/IMC 资产。
	if (MoveAction != nullptr)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARPGPlayerCharacter::Move);
	}

	if (LookAction != nullptr)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARPGPlayerCharacter::Look);
	}

	if (JumpAction != nullptr)
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}

	if (InteractAction != nullptr)
	{
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ARPGPlayerCharacter::Interact);
	}

	if (PrimaryActionInput != nullptr)
	{
		EnhancedInputComponent->BindAction(PrimaryActionInput, ETriggerEvent::Started, this, &ARPGPlayerCharacter::PrimaryAction);
	}

	if (SprintAction != nullptr)
	{
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ARPGPlayerCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ARPGPlayerCharacter::StopSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ARPGPlayerCharacter::StopSprint);
	}

	if (ZoomAction != nullptr)
	{
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ARPGPlayerCharacter::Zoom);
	}
}

void ARPGPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ARPGPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxisVector.X * MouseLookSensitivity);
	AddControllerPitchInput(LookAxisVector.Y * MouseLookSensitivity);
}

void ARPGPlayerCharacter::Zoom(const FInputActionValue& Value)
{
	const float ZoomValue = Value.Get<float>();
	if (FMath::IsNearlyZero(ZoomValue))
	{
		return;
	}

	CameraDistance = FMath::Clamp(CameraDistance - ZoomValue * ZoomStep, MinCameraDistance, MaxCameraDistance);
	CameraBoom->TargetArmLength = CameraDistance;
}

void ARPGPlayerCharacter::StartSprint()
{
	if (!bCanDash)
	{
		return;
	}

	bCanDash = false;

	FVector DashDirection = GetLastMovementInputVector();
	if (!bDashUseInputDirection || DashDirection.IsNearlyZero())
	{
		const FRotator ControlRotation = Controller != nullptr ? Controller->GetControlRotation() : GetActorRotation();
		const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
		DashDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	}

	DashDirection.Z = 0.0f;
	DashDirection.Normalize();

	UAnimSequenceBase* AnimationToPlay = DashAnimation;
	const bool bUseRootMotionDash = bDashUseRootMotionAnimation && AnimationToPlay != nullptr;
	if (!bUseRootMotionDash && IsLikelyRootMotionDashAnimation(AnimationToPlay))
	{
		UE_LOG(LogTemp, Warning, TEXT("DashAnimation 指向 RootMotion 资源：%s。当前闪避位移由 LaunchCharacter 控制，改播 DashFallbackAnimation 以避免 Mesh 回弹。"), *GetNameSafe(AnimationToPlay));
		AnimationToPlay = DashFallbackAnimation;
	}

	if (bUseRootMotionDash)
	{
		// RootMotion 动画通常沿角色前方位移；播放前把角色朝向本次闪避方向，避免侧向输入时动画和位移方向不一致。
		SetActorRotation(DashDirection.Rotation());

		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			// RootMotion 模式下不再叠加当前移动速度，让动画本身决定这次闪避的位移曲线。
			Movement->StopMovementImmediately();
		}
	}

	if (AnimationToPlay != nullptr)
	{
		// 原型阶段可以直接指定 AnimSequence；需要通知、无敌帧或位移曲线时，也可以把 DashAnimation 换成正式 AnimMontage。
		if (UAnimMontage* DashMontageAsset = Cast<UAnimMontage>(AnimationToPlay))
		{
			PlayAnimMontage(DashMontageAsset, DashAnimationPlayRate);
		}
		else
		{
			UAnimMontage* DashMontage = UAnimMontage::CreateSlotAnimationAsDynamicMontage(
				AnimationToPlay,
				DashAnimationSlotName,
				DashAnimationBlendIn,
				DashAnimationBlendOut,
				DashAnimationPlayRate);

			PlayAnimMontage(DashMontage);
		}
	}

	TWeakObjectPtr<ARPGPlayerCharacter> WeakThis(this);
	if (!bUseRootMotionDash)
	{
		LaunchCharacter(DashDirection * DashStrength, true, false);

		FTimerHandle DashStopTimerHandle;
		GetWorldTimerManager().SetTimer(DashStopTimerHandle, [WeakThis]()
		{
			if (!WeakThis.IsValid())
			{
				return;
			}

			UCharacterMovementComponent* Movement = WeakThis->GetCharacterMovement();
			if (Movement == nullptr)
			{
				return;
			}

			const FVector CurrentVelocity = Movement->Velocity;
			Movement->Velocity = FVector(0.0f, 0.0f, CurrentVelocity.Z);
		}, DashDuration, false);
	}

	FTimerHandle DashCooldownTimerHandle;
	GetWorldTimerManager().SetTimer(DashCooldownTimerHandle, [WeakThis]()
	{
		if (WeakThis.IsValid())
		{
			WeakThis->bCanDash = true;
		}
	}, DashCooldown, false);
}

void ARPGPlayerCharacter::StopSprint()
{
	// 闪避是一次性动作，松开按键不再控制速度。
}

bool ARPGPlayerCharacter::IsLikelyRootMotionDashAnimation(const UAnimSequenceBase* Animation) const
{
	if (Animation == nullptr)
	{
		return false;
	}

	const FString AnimationPath = Animation->GetPathName().ToLower();
	return AnimationPath.Contains(TEXT("/rootmotion/")) || AnimationPath.Contains(TEXT("/root_motion/"));
}

void ARPGPlayerCharacter::Interact()
{
	// 原型 001 的交互入口。后续只负责发起交互查询，不在角色里写具体 NPC/物品逻辑。
}

void ARPGPlayerCharacter::PrimaryAction()
{
	// 原型 001 的基础行动入口。后续转发给 CombatComponent，而不是在角色里直接结算伤害。
}
