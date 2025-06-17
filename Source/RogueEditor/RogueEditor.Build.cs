using UnrealBuildTool;

public class RogueEditor : ModuleRules
{
	public RogueEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"DataValidation",
			"BlueprintGraph",
			"ActionRoguelike"
		});
	}
}