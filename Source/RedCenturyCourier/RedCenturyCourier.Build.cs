// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RedCenturyCourier : ModuleRules
{
	public RedCenturyCourier(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		// Core
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
	
		// AI
		PublicDependencyModuleNames.AddRange(new string[] { "AIModule", "GameplayTasks", "NavigationSystem" });
	
		// JSON - we use this for save/load of user settings data
		PublicDependencyModuleNames.AddRange(new string[] { "Json", "JsonUtilities" });
		
		// Enhanced input - UE's new input system
		PublicDependencyModuleNames.AddRange(new string[] { "EnhancedInput"/*, "InputBlueprintNodes", "BlueprintGraph"*/ });

		// ModularNavigation
		PublicDependencyModuleNames.AddRange(new string[] { "ModularNavigation" });

		// user input
		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayTags" });
		
		// GAS - see https://github.com/tranek/GASDocumentation#3-setting-up-a-project-using-gas
		// PrivateDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "GameplayTags", "GameplayTasks" });
		
		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
