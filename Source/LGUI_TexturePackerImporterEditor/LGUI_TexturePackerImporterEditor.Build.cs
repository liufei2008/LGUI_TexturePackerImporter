// Copyright 2019-2021 LexLiu. All Rights Reserved.

using UnrealBuildTool;

public class LGUI_TexturePackerImporterEditor : ModuleRules
{
	public LGUI_TexturePackerImporterEditor(ReadOnlyTargetRules Target) : base(Target)
    {
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Slate",
                "SlateCore",
                "Engine",
                "UnrealEd",
                "PropertyEditor",
                "RenderCore",
                "RHI",
				"LGUI_TexturePackerImporter",
				"LGUI",
                "LGUIEditor",
                "LevelEditor",
                "Projects",
                "EditorWidgets",
                "DesktopPlatform",
                "ImageWrapper",
                "AssetTools",//Asset editor
				"SharedSettingsWidgets",
				"Json",
				"JsonUtilities",
				// ... add other public dependencies that you statically link with here ...
                
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "EditorStyle",
				// ... add private dependencies that you statically link with here ...	

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
