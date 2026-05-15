// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/RPGPlayerController.h"

#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "UObject/ConstructorHelpers.h"

ARPGPlayerController::ARPGPlayerController()
{
	bShowMouseCursor = false;

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

	// The asset is still named IA_Sprint, while C++ uses the gameplay term Dash.
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

	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	SetInputMode(FInputModeGameOnly());
	SetIgnoreLookInput(false);
	SetIgnoreMoveInput(false);

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (InputSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RPGPlayerController could not get Enhanced Input LocalPlayerSubsystem."));
		return;
	}

	if (bForceRuntimeExplorationMapping)
	{
		if (UInputMappingContext* RuntimeExplorationMappingContext = CreateRuntimeExplorationMappingContext())
		{
			InputSubsystem->AddMappingContext(RuntimeExplorationMappingContext, ExplorationMappingPriority);
			UE_LOG(LogTemp, Warning, TEXT("RPGPlayerController is forcing runtime exploration input mappings; IMC_Exploration edits are bypassed."));
		}

		return;
	}

	if (ExplorationMappingContext != nullptr)
	{
		InputSubsystem->AddMappingContext(ExplorationMappingContext, ExplorationMappingPriority);

		if (!HasRequiredExplorationMappings(ExplorationMappingContext))
		{
			UE_LOG(LogTemp, Warning, TEXT("IMC_Exploration is missing required prototype mappings."));
			if (bUseRuntimeMappingFallback)
			{
				if (UInputMappingContext* RuntimeExplorationMappingContext = CreateRuntimeExplorationMappingContext())
				{
					InputSubsystem->AddMappingContext(RuntimeExplorationMappingContext, ExplorationMappingPriority + 1);
					UE_LOG(LogTemp, Warning, TEXT("Added runtime exploration input fallback above IMC_Exploration."));
				}
			}
		}

		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ExplorationMappingContext is not set on RPGPlayerController."));
	if (!bUseRuntimeMappingFallback)
	{
		return;
	}

	if (UInputMappingContext* RuntimeExplorationMappingContext = CreateRuntimeExplorationMappingContext())
	{
		InputSubsystem->AddMappingContext(RuntimeExplorationMappingContext, ExplorationMappingPriority);
		UE_LOG(LogTemp, Warning, TEXT("Using runtime exploration input fallback because ExplorationMappingContext is missing."));
	}
}

UInputMappingContext* ARPGPlayerController::CreateRuntimeExplorationMappingContext()
{
	if (MoveAction == nullptr || LookAction == nullptr || JumpAction == nullptr || InteractAction == nullptr || PrimaryActionInput == nullptr || DashAction == nullptr || ZoomAction == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create runtime input mapping: one or more InputAction assets are missing."));
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

	// MoveAction is Axis2D: W/S write Y and A/D write X.
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

bool ARPGPlayerController::HasActionKeyMapping(const UInputMappingContext* MappingContext, const UInputAction* Action, FKey Key) const
{
	if (MappingContext == nullptr || Action == nullptr)
	{
		return false;
	}

	for (const FEnhancedActionKeyMapping& Mapping : MappingContext->GetMappings())
	{
		if (Mapping.Action == Action && Mapping.Key == Key)
		{
			return true;
		}
	}

	return false;
}

bool ARPGPlayerController::HasActionKeyMappingWithModifiers(const UInputMappingContext* MappingContext, const UInputAction* Action, FKey Key, bool bRequiresNegate, bool bRequiresSwizzle, const FVector* RequiredScalar) const
{
	if (MappingContext == nullptr || Action == nullptr)
	{
		return false;
	}

	for (const FEnhancedActionKeyMapping& Mapping : MappingContext->GetMappings())
	{
		if (Mapping.Action != Action || Mapping.Key != Key)
		{
			continue;
		}

		if (bRequiresNegate && !MappingHasNegateModifier(Mapping))
		{
			continue;
		}

		if (bRequiresSwizzle && !MappingHasSwizzleModifier(Mapping))
		{
			continue;
		}

		if (RequiredScalar != nullptr && !MappingHasScalarModifier(Mapping, *RequiredScalar))
		{
			continue;
		}

		return true;
	}

	return false;
}

bool ARPGPlayerController::MappingHasNegateModifier(const FEnhancedActionKeyMapping& Mapping) const
{
	for (const TObjectPtr<UInputModifier>& Modifier : Mapping.Modifiers)
	{
		if (Modifier != nullptr && Modifier->IsA<UInputModifierNegate>())
		{
			return true;
		}
	}

	return false;
}

bool ARPGPlayerController::MappingHasSwizzleModifier(const FEnhancedActionKeyMapping& Mapping) const
{
	for (const TObjectPtr<UInputModifier>& Modifier : Mapping.Modifiers)
	{
		const UInputModifierSwizzleAxis* SwizzleModifier = Cast<UInputModifierSwizzleAxis>(Modifier);
		if (SwizzleModifier != nullptr && SwizzleModifier->Order == EInputAxisSwizzle::YXZ)
		{
			return true;
		}
	}

	return false;
}

bool ARPGPlayerController::MappingHasScalarModifier(const FEnhancedActionKeyMapping& Mapping, const FVector& RequiredScalar) const
{
	for (const TObjectPtr<UInputModifier>& Modifier : Mapping.Modifiers)
	{
		const UInputModifierScalar* ScalarModifier = Cast<UInputModifierScalar>(Modifier);
		if (ScalarModifier != nullptr && ScalarModifier->Scalar.Equals(RequiredScalar, KINDA_SMALL_NUMBER))
		{
			return true;
		}
	}

	return false;
}

bool ARPGPlayerController::HasRequiredExplorationMappings(const UInputMappingContext* MappingContext) const
{
	const FVector MouseLookYawScalar(1.0f, 0.0f, 0.0f);
	const FVector MouseLookPitchScalar(0.0f, -1.0f, 0.0f);

	return
		HasActionKeyMappingWithModifiers(MappingContext, MoveAction, EKeys::W, false, true, nullptr) &&
		HasActionKeyMappingWithModifiers(MappingContext, MoveAction, EKeys::S, true, true, nullptr) &&
		HasActionKeyMappingWithModifiers(MappingContext, MoveAction, EKeys::A, true, false, nullptr) &&
		HasActionKeyMapping(MappingContext, MoveAction, EKeys::D) &&
		HasActionKeyMapping(MappingContext, MoveAction, EKeys::Gamepad_Left2D) &&
		HasActionKeyMappingWithModifiers(MappingContext, LookAction, EKeys::MouseX, false, false, &MouseLookYawScalar) &&
		HasActionKeyMappingWithModifiers(MappingContext, LookAction, EKeys::MouseY, false, true, &MouseLookPitchScalar) &&
		HasActionKeyMapping(MappingContext, LookAction, EKeys::Gamepad_Right2D) &&
		HasActionKeyMapping(MappingContext, JumpAction, EKeys::SpaceBar) &&
		HasActionKeyMapping(MappingContext, JumpAction, EKeys::Gamepad_FaceButton_Bottom) &&
		HasActionKeyMapping(MappingContext, InteractAction, EKeys::E) &&
		HasActionKeyMapping(MappingContext, PrimaryActionInput, EKeys::LeftMouseButton) &&
		HasActionKeyMapping(MappingContext, DashAction, EKeys::LeftShift) &&
		HasActionKeyMapping(MappingContext, ZoomAction, EKeys::MouseWheelAxis);
}
