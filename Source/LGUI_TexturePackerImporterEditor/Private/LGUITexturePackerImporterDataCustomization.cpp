// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUITexturePackerImporterDataCustomization.h"
#include "LGUI_TexturePackerImporterEditorPrivatePCH.h"
#include "MISC/FileHelper.h"
#include "Widget/LGUIFileBrowser.h"
#include "Core/Actor/LGUIManagerActor.h"

#include "AssetRegistryModule.h"
#include "Factories.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "ImageUtils.h"
#include "Misc/Parse.h"
#include "SHyperlinkLaunchURL.h"
#include "ObjectTools.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

#include "AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "LGUITexturePackerImporterDataCustomization"


TSharedRef<IDetailCustomization> FLGUITexturePackerImporterDataCustomization::MakeInstance()
{
	return MakeShareable(new FLGUITexturePackerImporterDataCustomization);
}

void FLGUITexturePackerImporterDataCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULGUITexturePackerImporterData>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUI_TexturePackerImporterEditor, Log, TEXT("Get TargetScript is null"));
		return;
	}

	DetailBuilder.HideProperty("fntFileSourcePath");
	IDetailCategoryBuilder& createCategory = DetailBuilder.EditCategory("Create");
	createCategory.AddCustomRow(LOCTEXT("Tips", "Tips"))
		.WholeRowContent()
		[
			SNew(SBox)
			.HeightOverride(40)
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Use \"TexturePacker\" to export .json (array) and .png file. \"TexturePacker\" can be found here:"))
					.AutoWrapText(true)
				]
				+SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.HAlign(HAlign_Center)
						[
							SNew(SHyperlinkLaunchURL, TEXT("https://www.codeandweb.com/texturepacker"))
							.Text(LOCTEXT("Bitmap Font Generator", "https://www.codeandweb.com/texturepacker"))
						]
					]
				]
			]
		];

	auto jsonFilePathHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUITexturePackerImporterData, jsonFilePath));
	FString fntFileSourcePath;
	jsonFilePathHandle->GetValue(fntFileSourcePath);
	auto targetFolderHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUITexturePackerImporterData, targetFolder));
	//auto deleteSpriteIfNotPresentHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUITexturePackerImporterData, deleteSpriteIfNotPresent));
	createCategory.AddCustomRow(LOCTEXT("JsonSourceFile","Json File Path"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("JsonSourceFile", "Json File Path"))
		]
		.ValueContent()
		.MinDesiredWidth(300)
		[	
			SNew(SLGUIFileBrowser)
			.FolderPath(this, &FLGUITexturePackerImporterDataCustomization::OnGetFontFilePath)
			.DialogTitle(TEXT("Browse for a json file"))
			.DefaultFileName("atlas.json")
			.Filter("TexturePacker exported file|*.json")
			.OnFilePathChanged(this, &FLGUITexturePackerImporterDataCustomization::OnPathTextChanged, jsonFilePathHandle)
			.OnFilePathCommitted(this, &FLGUITexturePackerImporterDataCustomization::OnPathTextCommitted, jsonFilePathHandle)
		];

	createCategory.AddCustomRow(LOCTEXT("Create", "Create"))
		.WholeRowContent()
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked(this, &FLGUITexturePackerImporterDataCustomization::OnCreateButtonClicked
				, jsonFilePathHandle
				, targetFolderHandle
			)
			.Text(LOCTEXT("Create", "Create"))
		];

	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULGUITexturePackerImporterData, jsonFilePath));
}

FText FLGUITexturePackerImporterDataCustomization::OnGetFontFilePath()const
{
	auto& fileManager = IFileManager::Get();
	return FText::FromString(TargetScriptPtr->jsonFilePath.IsEmpty() ? fileManager.GetFilenameOnDisk(*FPaths::ProjectDir()) : TargetScriptPtr->jsonFilePath);
}

void FLGUITexturePackerImporterDataCustomization::OnPathTextChanged(const FString& InString, TSharedRef<IPropertyHandle> PropertyHandle)
{
	PropertyHandle->SetValue(InString);
}
void FLGUITexturePackerImporterDataCustomization::OnPathTextCommitted(const FString& InString, TSharedRef<IPropertyHandle> PropertyHandle)
{
	PropertyHandle->SetValue(InString);
}

FReply FLGUITexturePackerImporterDataCustomization::OnCreateButtonClicked(
	TSharedRef<IPropertyHandle> jsonFilePropertyHandle
	, TSharedRef<IPropertyHandle> targetFolderPropertyHandle
)
{
	FString jsonFilePath;
	jsonFilePropertyHandle->GetValue(jsonFilePath);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.FileExists(*jsonFilePath))
	{
		UE_LOG(LGUI_TexturePackerImporterEditor, Error, TEXT("[LGUI TexturePackerImporter] Json file: [%s] not exist!"), *jsonFilePath);
		return FReply::Handled();
	}
	FString jsonString;
	if (!FFileHelper::LoadFileToString(jsonString, *jsonFilePath))
	{
		UE_LOG(LGUI_TexturePackerImporterEditor, Error, TEXT("[LGUI TexturePackerImporter] Load file: [%s] failed!"), *jsonFilePath);
		return FReply::Handled();
	}

	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
	TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(jsonString);
	if (!FJsonSerializer::Deserialize(jsonReader, jsonObject) && jsonObject.IsValid())
	{
		UE_LOG(LGUI_TexturePackerImporterEditor, Error, TEXT("[LGUI TexturePackerImporter] Deserialize json file: [%s] failed!"), *jsonFilePath);
		return FReply::Handled();
	}
	auto metaJObject = jsonObject->GetObjectField("meta");
	TargetScriptPtr->version = metaJObject->GetStringField("version");
	auto metaImage = metaJObject->GetStringField("image");
	auto metaSizeJObject = metaJObject->GetObjectField("size");
	auto metaTextureWidth = metaSizeJObject->GetIntegerField("w");
	auto metaTextureHeight = metaSizeJObject->GetIntegerField("h");
	auto metaFormat = metaJObject->GetStringField("format");
	auto textureFilePath = FPaths::GetPath(jsonFilePath) + "/" + metaImage;
	if (!PlatformFile.FileExists(*textureFilePath))
	{
		UE_LOG(LGUI_TexturePackerImporterEditor, Error, TEXT("[LGUI TexturePackerImporter] Texture file: [%s] not exist!"), *textureFilePath);
		return FReply::Handled();
	}
	if (metaFormat != "RGBA8888")
	{
		UE_LOG(LGUI_TexturePackerImporterEditor, Error, TEXT("[LGUI TexturePackerImporter] Texture format: [%s] not support!"), *metaFormat);
		return FReply::Handled();
	}

	FString saveFolder, defaultFolder;
	targetFolderPropertyHandle->GetValue(defaultFolder);
	if (!GetFolderToSaveSpriteDatas(saveFolder, defaultFolder))
	{
		return FReply::Handled();
	}
	targetFolderPropertyHandle->SetValue(saveFolder);
	float texFullWidthReciprocal = 1.0f / metaTextureWidth;
	float texFullHeightReciprocal = 1.0f / metaTextureHeight;
	auto framesJObject = jsonObject->GetArrayField("frames");
	bool hasRotated = false;
	TArray<SpriteDateStruct> spriteDataArray;
	for (int i = 0; i < framesJObject.Num(); i++)
	{
		auto itemJObject = framesJObject[i]->AsObject();
		SpriteDateStruct spriteData;
		spriteData.name = itemJObject->GetStringField("filename");
		auto frameJObject = itemJObject->GetObjectField("frame");
		spriteData.frame_x = frameJObject->GetIntegerField("x");
		spriteData.frame_y = frameJObject->GetIntegerField("y");
		spriteData.frame_w = frameJObject->GetIntegerField("w");
		spriteData.frame_h = frameJObject->GetIntegerField("h");
		auto rotated = itemJObject->GetBoolField("rotated");
		auto trimmed = itemJObject->GetBoolField("trimmed");
		auto spriteSourceSizeJObject = itemJObject->GetObjectField("spriteSourceSize");
		spriteData.spriteSourceSize_x = spriteSourceSizeJObject->GetIntegerField("x");
		spriteData.spriteSourceSize_y = spriteSourceSizeJObject->GetIntegerField("y");
		spriteData.spriteSourceSize_w = spriteSourceSizeJObject->GetIntegerField("w");
		spriteData.spriteSourceSize_h = spriteSourceSizeJObject->GetIntegerField("h");
		auto sourceSizeJOjbect = itemJObject->GetObjectField("sourceSize");
		spriteData.sourceSize_w = sourceSizeJOjbect->GetIntegerField("w");
		spriteData.sourceSize_h = sourceSizeJOjbect->GetIntegerField("h");
		spriteDataArray.Add(spriteData);
		if (rotated)
		{
			FMessageDialog::Open(EAppMsgType::Ok
				, FText::FromString(FString::Printf(TEXT("Detect rotated sprite, this is not supported!\
\nPlease uncheck the \"Allow rotation\" in TexturePacker."))));
			return FReply::Handled();
		}
		if (trimmed)
		{
			FMessageDialog::Open(EAppMsgType::Ok
				, FText::FromString(FString::Printf(TEXT("Detect trimmed sprite, this is not supported!\
\nPlease change the \"Trim mode\" to \"None\" in TexturePacker."))));
			return FReply::Handled();
		}
	}
	auto prevSprites = TargetScriptPtr->sprites;
	TargetScriptPtr->sprites.Empty();
	for (int i = 0; i < spriteDataArray.Num(); i++)
	{
		auto spriteDataObj = CreateSpriteData(spriteDataArray[i], texFullWidthReciprocal, texFullHeightReciprocal, saveFolder);
		TargetScriptPtr->sprites.Add(spriteDataObj);
	}

	UPackage* texturePackage;
	FString texturePackageName;
	if (IsValid(TargetScriptPtr->atlasTexture))
	{
		texturePackage = TargetScriptPtr->atlasTexture->GetPackage();
		texturePackageName = TargetScriptPtr->atlasTexture->GetPathName();
		int indexOfDot = 0;
		if (texturePackageName.FindLastChar('.', indexOfDot))
		{
			texturePackageName = texturePackageName.Left(indexOfDot);
		}
	}
	else
	{
		texturePackageName = TargetScriptPtr->GetPathName();
		int lastCharIndex = 0;
		if (texturePackageName.FindLastChar('/', lastCharIndex))
		{
			texturePackageName = texturePackageName.Left(lastCharIndex + 1);
		}
		texturePackageName.Append(TargetScriptPtr->GetName()).Append(TEXT("_Texture"));
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 26
		texturePackage = CreatePackage(*texturePackageName);
#else
		texturePackage = CreatePackage(NULL, *texturePackageName);
#endif
	}

	bool canceled = false;
	UTextureFactory* Factory = NewObject<UTextureFactory>(UTextureFactory::StaticClass());
	Factory->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	Factory->bAlphaToOpacity = true;
	Factory->bDeferCompression = true;
	auto importedAsset = Factory->ImportObject(UTexture2D::StaticClass(), texturePackage, FName(*FPaths::GetCleanFilename(texturePackageName)), RF_Public | RF_Standalone, textureFilePath, nullptr, canceled);
	if (canceled)
	{
		return FReply::Handled();
	}
	TargetScriptPtr->atlasTexture = Cast<UTexture2D>(importedAsset);
	TargetScriptPtr->atlasTexture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	TargetScriptPtr->atlasTexture->UpdateResource();
	FAssetRegistryModule::AssetCreated(TargetScriptPtr->atlasTexture);
	texturePackage->SetDirtyFlag(true);

	TargetScriptPtr->MarkPackageDirty();
	ULGUIEditorManagerObject::RefreshAllUI();

#if 0
	//@todo: search all ULGUITexturePackerSpriteData, if it is not contained by any atlas then collect and delete it
	//if (TargetScriptPtr->deleteSpriteIfNotPresent)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		// Need to do this if running in the editor with -game to make sure that the assets in the following path are available
		TArray<FString> PathsToScan;
		PathsToScan.Add(TEXT("/Game/"));
		AssetRegistry.ScanPathsSynchronous(PathsToScan);

		// Get asset in path
		TArray<FAssetData> ScriptAssetList;
		AssetRegistry.GetAssetsByPath(FName("/Game/"), ScriptAssetList, /*bRecursive=*/true);

		// Ensure all assets are loaded
		for (const FAssetData& Asset : ScriptAssetList)
		{
			// Gets the loaded asset, loads it if necessary
			if (Asset.AssetClass == TEXT("LGUITexturePackerSpriteData"))
			{
				auto assetObject = Asset.GetAsset();
				if (auto prefab = Cast<ULGUITexturePackerSpriteData>(assetObject))
				{
					
				}
			}
		}
	}
#endif

	return FReply::Handled();
}

bool FLGUITexturePackerImporterDataCustomization::GetFolderToSaveSpriteDatas(FString& OutFolderName, const FString& InDefaultFolder)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		auto defaultFolder = FPaths::Combine(FPaths::ProjectContentDir(), InDefaultFolder);
		if (
			DesktopPlatform->OpenDirectoryDialog(
				FSlateApplication::Get().FindBestParentWindowHandleForDialogs(FSlateApplication::Get().GetGameViewport()),
				TEXT("LGUI will generate a lot of sprite-data assets depend on json file, please choose a folder to save these assets, must inside Content folder"),
				defaultFolder,
				OutFolderName
			)
			)
		{
			if (OutFolderName.StartsWith(FPaths::ProjectContentDir()))
			{
				OutFolderName.RemoveFromStart(FPaths::ProjectContentDir());
				return true;
			}
			else
			{
				FMessageDialog::Open(EAppMsgType::Ok
					, FText::FromString(FString::Printf(TEXT("You must select a folder inside Content folder!"))));
			}
		}
	}
	return false;
}
ULGUITexturePackerSpriteData* FLGUITexturePackerImporterDataCustomization::CreateSpriteData(SpriteDateStruct SpriteData, float texFullWidthReciprocal, float texFullHeightReciprocal, const FString& FolderPath)
{
	ULGUITexturePackerSpriteData* Result = nullptr;

	int lastIndexOfDot;
	if (SpriteData.name.FindLastChar('.', lastIndexOfDot))
	{
		SpriteData.name = SpriteData.name.Left(lastIndexOfDot);
	}
	FString packageName = TEXT("/Game/") + FolderPath + "/" + SpriteData.name;
	FString assetName = packageName + "." + FPaths::GetBaseFilename(SpriteData.name);
	Result = LoadObject<ULGUITexturePackerSpriteData>(NULL, *assetName);
	bool isCreated = false;
	if (!IsValid(Result))
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 26
		UPackage* package = CreatePackage(*packageName);
#else
		UPackage* package = CreatePackage(NULL, *packageName);
#endif
		package->FullyLoad();
		FString fileName = FPaths::GetBaseFilename(SpriteData.name);
		Result = NewObject<ULGUITexturePackerSpriteData>(package, ULGUITexturePackerSpriteData::StaticClass(), *fileName, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
		isCreated = true;
	}
	auto& spriteInfo = Result->spriteInfo;
	spriteInfo.width = SpriteData.frame_w;
	spriteInfo.height = SpriteData.frame_h;
	spriteInfo.ApplyUV(SpriteData.frame_x, SpriteData.frame_y, SpriteData.frame_w, SpriteData.frame_h, texFullWidthReciprocal, texFullHeightReciprocal);
	spriteInfo.ApplyBorderUV(texFullWidthReciprocal, texFullHeightReciprocal);

	Result->importer = TargetScriptPtr.Get();
	Result->MarkPackageDirty();
	if (isCreated)
	{
		FAssetRegistryModule::AssetCreated(Result);
	}
	return Result;
}
#undef LOCTEXT_NAMESPACE