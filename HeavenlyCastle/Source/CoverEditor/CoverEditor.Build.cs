using UnrealBuildTool;

public class CoverEditor : ModuleRules
{
    public CoverEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",
            "Slate",
            "SlateCore",
            "EditorFramework",
            "ComponentVisualizers",
            "ApplicationCore",
            "InputCore",
            "Cover"
        });
    }
}
