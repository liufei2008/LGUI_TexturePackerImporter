// Copyright 2021-present LexLiu. All Rights Reserved.

#include "LGUI_TexturePackerImporterEditorModule.h"
#include "LGUI_TexturePackerImporterEditorPrivatePCH.h"

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "LGUI_TexturePackerImporterEditorModule"
DEFINE_LOG_CATEGORY(LGUI_TexturePackerImporterEditor)

void FLGUI_TexturePackerImporterEditorModule::StartupModule()
{		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	//register detail editor
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(ULGUITexturePackerImporterData::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUITexturePackerImporterDataCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(ULGUITexturePackerSpriteData::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUITexturePackerSpriteDataCustomization::MakeInstance));
	}
	//register asset
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		//register AssetCategory
		EAssetTypeCategories::Type LGUIAssetCategoryBit = AssetTools.FindAdvancedAssetCategory(FName(TEXT("LGUI")));
		if (LGUIAssetCategoryBit == EAssetTypeCategories::Misc)
		{
			LGUIAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("LGUI")), LOCTEXT("LGUIAssetCategory", "LGUI"));
		}

		TPDataTypeAction = MakeShareable(new FLGUITexturePackerImporterDataTypeAction(LGUIAssetCategoryBit));
		AssetTools.RegisterAssetTypeActions(TPDataTypeAction.ToSharedRef());
	}
}

void FLGUI_TexturePackerImporterEditorModule::ShutdownModule()
{
	if (UObjectInitialized() && FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(ULGUITexturePackerImporterData::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(ULGUITexturePackerSpriteData::StaticClass()->GetFName());
	}

	if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetTools")))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(TPDataTypeAction.ToSharedRef());
	}
}
	
IMPLEMENT_MODULE(FLGUI_TexturePackerImporterEditorModule, LGUI_TexturePackerImporterEditor)

#undef LOCTEXT_NAMESPACE