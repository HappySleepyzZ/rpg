// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RPGPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
struct FEnhancedActionKeyMapping;

UCLASS()
class RPG260514_API ARPGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARPGPlayerController();

protected:
	virtual void BeginPlay() override;

private:
	UInputMappingContext* CreateRuntimeExplorationMappingContext();
	bool HasActionKeyMapping(const UInputMappingContext* MappingContext, const UInputAction* Action, FKey Key) const;
	bool HasActionKeyMappingWithModifiers(const UInputMappingContext* MappingContext, const UInputAction* Action, FKey Key, bool bRequiresNegate, bool bRequiresSwizzle, const FVector* RequiredScalar) const;
	bool MappingHasNegateModifier(const FEnhancedActionKeyMapping& Mapping) const;
	bool MappingHasSwizzleModifier(const FEnhancedActionKeyMapping& Mapping) const;
	bool MappingHasScalarModifier(const FEnhancedActionKeyMapping& Mapping, const FVector& RequiredScalar) const;
	bool HasRequiredExplorationMappings(const UInputMappingContext* MappingContext) const;

	// Exploration input context: movement, camera, jump, interact, primary action, dash and zoom.
	// Later combat input should add a separate context instead of changing character input code.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> ExplorationMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	int32 ExplorationMappingPriority = 0;

	// Use runtime-generated mappings only when the asset IMC is missing or incomplete.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	bool bUseRuntimeMappingFallback = true;

	// Emergency prototype override. Leave false by default so IMC_Exploration edits are honored.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	bool bForceRuntimeExplorationMapping = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PrimaryActionInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true", DisplayName = "Dash Action"))
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ZoomAction;
};
