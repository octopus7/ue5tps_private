// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HeavenlyCastle : ModuleRules
{
	public HeavenlyCastle(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            // Public headers reference these modules
            "UMG",            // UUserWidget, Widget types in public UI headers
            "Niagara",        // UNiagaraSystem used in public headers
            "AIModule",       // AAIController in public headers
            "NavigationSystem" // UNavigationSystemV1 used in AI controller
        });

        PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
