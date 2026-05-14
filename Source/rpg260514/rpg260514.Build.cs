// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class rpg260514 : ModuleRules
{
	public rpg260514(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		PublicIncludePaths.AddRange(new string[] {
			"rpg260514",
			"rpg260514/AI",
			"rpg260514/Character",
			"rpg260514/Combat",
			"rpg260514/Core",
			"rpg260514/Data",
			"rpg260514/Interaction",
			"rpg260514/Inventory",
			"rpg260514/Quest",
			"rpg260514/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
