// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/RPGPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/BlendSpace.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "UObject/ConstructorHelpers.h"

ARPGPlayerCharacter::ARPGPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	Movement->JumpZVelocity = 500.0f;
	Movement->AirControl = 0.35f;
	Movement->MaxWalkSpeed = 500.0f;
	Movement->MinAnalogWalkSpeed = 20.0f;
	Movement->BrakingDecelerationWalking = 2000.0f;
	Movement->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
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

	static ConstructorHelpers::FObjectFinder<UBlendSpace> LocomotionBlendSpaceAsset(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/BS_Idle_Walk_Run.BS_Idle_Walk_Run"));
	PrototypeLocomotionBlendSpace = LocomotionBlendSpaceAsset.Object;
}

void ARPGPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (PrototypeLocomotionBlendSpace != nullptr)
	{
		// 先不用复杂官方 ABP，直接让 Mesh 进入单节点动画模式播放 BlendSpace。
		// 这样原型阶段可以清楚验证移动动作，后续再替换成正式动画蓝图。
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		GetMesh()->SetAnimation(PrototypeLocomotionBlendSpace);
		GetMesh()->Play(true);
	}
}

void ARPGPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdatePrototypeLocomotionAnimation();
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
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ARPGPlayerCharacter::Interact()
{
	// 原型 001 的交互入口。后续只负责发起交互查询，不在角色里写具体 NPC/物品逻辑。
}

void ARPGPlayerCharacter::PrimaryAction()
{
	// 原型 001 的基础行动入口。后续转发给 CombatComponent，而不是在角色里直接结算伤害。
}

void ARPGPlayerCharacter::UpdatePrototypeLocomotionAnimation()
{
	if (PrototypeLocomotionBlendSpace == nullptr)
	{
		return;
	}

	UAnimSingleNodeInstance* SingleNodeInstance = GetMesh()->GetSingleNodeInstance();
	if (SingleNodeInstance == nullptr)
	{
		return;
	}

	const FVector Velocity = GetVelocity();
	const float GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Length();
	SingleNodeInstance->SetBlendSpacePosition(FVector(GroundSpeed, 0.0f, 0.0f));
}
