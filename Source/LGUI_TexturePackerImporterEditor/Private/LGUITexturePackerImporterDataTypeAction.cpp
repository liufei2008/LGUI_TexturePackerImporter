// Copyright 2021-present LexLiu. All Rights Reserved.

#include "LGUITexturePackerImporterDataTypeAction.h"
#include "LGUI_TexturePackerImporterEditorPrivatePCH.h"
#include "ContentBrowserModule.h"

#define LOCTEXT_NAMESPACE "LGUITexturePackerImporterDataTypeAction"

FLGUITexturePackerImporterDataTypeAction::FLGUITexturePackerImporterDataTypeAction(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FLGUITexturePackerImporterDataTypeAction::CanFilter()
{
	return true;
}

void FLGUITexturePackerImporterDataTypeAction::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FLGUITexturePackerImporterDataTypeAction::GetCategories()
{
	return assetType;
}

FText FLGUITexturePackerImporterDataTypeAction::GetName()const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_TexturePackerImporterDataAsset", "LGUI TexturePacker Importer");
}

UClass* FLGUITexturePackerImporterDataTypeAction::GetSupportedClass()const
{
	return ULGUITexturePackerImporterData::StaticClass();
}

FColor FLGUITexturePackerImporterDataTypeAction::GetTypeColor()const
{
	return FColor::White;
}

bool FLGUITexturePackerImporterDataTypeAction::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
