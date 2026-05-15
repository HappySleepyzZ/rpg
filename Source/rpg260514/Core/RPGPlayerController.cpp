// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/RPGPlayerController.h"

#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputCoreTypes.h"
#include "UObject/ConstructorHelpers.h"

ARPGPlayerController::ARPGPlayerController()
{
	bShowMouseCursor = false;

	// 探索输入上下文默认指向项目资产；蓝图子类仍可覆盖为测试或平台专用上下文。
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> ExplorationMappingAsset(TEXT("/Game/Input/IMC_Exploration.IMC_Exploration"));
	ExplorationMappingContext = ExplorationMappingAsset.Object;

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

	// 资产文件暂沿用 IA_Sprint，C++ 语义已统一为 Dash。后续重命名资产时同步调整输入脚本即可。
	static ConstructorHelpers::FObjectFinder<UInputAction> DashActionAsset(TEXT("/Game/Input/Actions/IA_Sprint.IA_Sprint"));
	DashAction = DashActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> ZoomActionAsset(TEXT("/Game/Input/Actions/IA_Zoom.IA_Zoom"));
	ZoomAction = ZoomActionAsset.Object;
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
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("尚未设置 ExplorationMappingContext。请在玩家控制器蓝图中指定 IMC_Exploration。"));
	if (!bUseRuntimeMappingFallback)
	{
		return;
	}

	if (UInputMappingContext* RuntimeExplorationMappingContext = CreateRuntimeExplorationMappingContext())
	{
		// 资产 IMC 缺失时才创建运行时映射，避免编辑器资产和代码各写一份按键导致重复触发。
		InputSubsystem->AddMappingContext(RuntimeExplorationMappingContext, ExplorationMappingPriority);
	}
}

UInputMappingContext* ARPGPlayerController::CreateRuntimeExplorationMappingContext()
{
	if (MoveAction == nullptr || LookAction == nullptr || JumpAction == nullptr || InteractAction == nullptr || PrimaryActionInput == nullptr || DashAction == nullptr || ZoomAction == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("创建运行时输入映射失败：存在未设置的 InputAction。"));
		return nullptr;
	}

	UInputMappingContext* RuntimeContext = NewObject<UInputMappingContext>(this, TEXT("RuntimeExplorationMappingContext"));

	auto AddNegateModifier = [RuntimeContext](FEnhancedActionKeyMapping& Mapping)
	{
		UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(RuntimeContext);
		Mapping.Modifiers.Add(Negate);
	};

	auto AddSwizzleModifier = [RuntimeContext](FEnhancedActionKeyMapping& Mapping)
	{
		UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(RuntimeContext);
		Swizzle->Order = EInputAxisSwizzle::YXZ;
		Mapping.Modifiers.Add(Swizzle);
	};

	auto AddScalarModifier = [RuntimeContext](FEnhancedActionKeyMapping& Mapping, const FVector& ScalarValue)
	{
		UInputModifierScalar* Scalar = NewObject<UInputModifierScalar>(RuntimeContext);
		Scalar->Scalar = ScalarValue;
		Mapping.Modifiers.Add(Scalar);
	};

	// 键盘移动：MoveAction 是 2D 轴。W/S 写入 Y，A/D 写入 X。
	AddSwizzleModifier(RuntimeContext->MapKey(MoveAction, EKeys::W));

	FEnhancedActionKeyMapping& MoveBackward = RuntimeContext->MapKey(MoveAction, EKeys::S);
	AddNegateModifier(MoveBackward);
	AddSwizzleModifier(MoveBackward);

	FEnhancedActionKeyMapping& MoveLeft = RuntimeContext->MapKey(MoveAction, EKeys::A);
	AddNegateModifier(MoveLeft);

	RuntimeContext->MapKey(MoveAction, EKeys::D);
	RuntimeContext->MapKey(MoveAction, EKeys::Gamepad_Left2D);

	FEnhancedActionKeyMapping& MouseLookYaw = RuntimeContext->MapKey(LookAction, EKeys::MouseX);
	AddScalarModifier(MouseLookYaw, FVector(1.0f, 0.0f, 0.0f));

	FEnhancedActionKeyMapping& MouseLookPitch = RuntimeContext->MapKey(LookAction, EKeys::MouseY);
	AddSwizzleModifier(MouseLookPitch);
	AddScalarModifier(MouseLookPitch, FVector(0.0f, -1.0f, 0.0f));

	RuntimeContext->MapKey(LookAction, EKeys::Gamepad_Right2D);

	RuntimeContext->MapKey(JumpAction, EKeys::SpaceBar);
	RuntimeContext->MapKey(JumpAction, EKeys::Gamepad_FaceButton_Bottom);
	RuntimeContext->MapKey(InteractAction, EKeys::E);
	RuntimeContext->MapKey(PrimaryActionInput, EKeys::LeftMouseButton);
	RuntimeContext->MapKey(DashAction, EKeys::LeftShift);
	RuntimeContext->MapKey(ZoomAction, EKeys::MouseWheelAxis);

	return RuntimeContext;
}
