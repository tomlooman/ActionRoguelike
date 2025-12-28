// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class ActionRoguelike : ModuleRules
{
	public ActionRoguelike(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		SetupIrisSupport(Target);
		
		// Simplify the include paths in our source files
		PublicIncludePaths.AddRange(
			new string[] {
				"ActionRoguelike"
			}
		);
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"AIModule", 
			"NavigationSystem",
			"GameplayTasks", 
			"UMG", 
			"GameplayTags", 
			"OnlineSubsystem", 
			"DeveloperSettings",
			"SignificanceManager",
			"EnhancedInput",
			"Niagara",
			"CoreOnline",
			"NetCore", 
			"StateTreeModule", 
			"GameplayStateTreeModule",
			"PropertyBindingUtils" // needed for property binding in state tree
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"MoviePlayer", // loading screen
			"RenderCore", // for PSO caching code access
			"AnimationBudgetAllocator"
		});
	}
}
