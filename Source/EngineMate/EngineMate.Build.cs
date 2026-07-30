// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EngineMate : ModuleRules
{
	public EngineMate(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "Source/Editor/Blutility/Private",
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				"Blutility", 
				"EditorScriptingUtilities", 
				"UMG", 
				"Niagara", 
				"AIModule", 
				"LevelSequence", 
				"IKRig", 
				"EnhancedInput", 
				"Engine", 
				"UnrealEd", 
				"PhysicsCore",
				"AssetTools",
				"ContentBrowser",
				"Projects", 
				"DataValidation",
				"DeveloperSettings"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"InteractiveToolsFramework",
				"EditorInteractiveToolsFramework", 
				"CinematicCamera",
				"Niagara",
				"UMGEditor",
				"UMG",
				"EditorScriptingUtilities",
				"ToolMenus",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
