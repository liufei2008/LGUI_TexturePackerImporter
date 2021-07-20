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

#define LOCTEXT_NAMESPACE "LGUITexturePackerImporterDataCustomization"



class TextureUtils
{
public:
	static UTexture2D* CreateTextureAsset(UTexture2D* OldTexture, FString PackageName, int Width, int Height, TArray<FColor>& Samples, TextureCompressionSettings CompressionSettings, TextureGroup LODGroup)
	{
		FCreateTexture2DParameters TexParams;
		TexParams.bUseAlpha = true;
		TexParams.CompressionSettings = CompressionSettings;
		TexParams.bDeferCompression = true;
		TexParams.bSRGB = false;
		TexParams.SourceGuidHash = FGuid::NewGuid();

		if (IsValid(OldTexture))
		{
			if (FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString(TEXT("A delete texture dialog will showup, just confirm delete.")))) == EAppReturnType::Ok)
			{
				ObjectTools::DeleteAssets({ OldTexture }, true);//if not show confirm dialog, then assets won't be deleted, don't know why
			}
		}
		auto Package = FindPackage(NULL, *PackageName);
		if (!IsValid(Package))
		{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 26
			Package = CreatePackage(*PackageName);
#else
			Package = CreatePackage(NULL, *PackageName);
#endif
		}
		UTexture2D* NewTexture = FImageUtils::CreateTexture2D(Width, Height, Samples, Package, FPaths::GetCleanFilename(PackageName), RF_Standalone | RF_Public, TexParams);
		NewTexture->LODGroup = LODGroup;
		NewTexture->DeferCompression = false;

		FAssetRegistryModule::AssetCreated(NewTexture);
		Package->SetDirtyFlag(true);

		return NewTexture;
	}

	static bool LoadTexture2DFromFile(const FString& ImagePath, EImageFormat ImageFormat, TArray<uint8>& ResultRawData, int& ResultWidth, int& ResultHeight)
	{
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);

		//Load From File
		TArray<uint8> RawFileData;
		if (!FFileHelper::LoadFileToArray(RawFileData, *ImagePath)) return NULL;

		//Create T2D!
		if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
		{
			ResultWidth = ImageWrapper->GetWidth();
			ResultHeight = ImageWrapper->GetHeight();
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 25
			return ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, ResultRawData);
#else
			const TArray<uint8>* UncompressedBGRA = NULL;
			if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
			{
				ResultRawData = *UncompressedBGRA;
				return true;
			}
#endif
		}

		return false;
	}
};


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
			.OnClicked(this, &FLGUITexturePackerImporterDataCustomization::OnCreateButtonClicked, jsonFilePathHandle)
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

FReply FLGUITexturePackerImporterDataCustomization::OnCreateButtonClicked(TSharedRef<IPropertyHandle> PropertyHandle)
{
	FString jsonFilePath;
	PropertyHandle->GetValue(jsonFilePath);
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

	int width, height;
	TArray<uint8> rawData;
	if (!TextureUtils::LoadTexture2DFromFile(textureFilePath, EImageFormat::PNG, rawData, width, height))
	{
		UE_LOG(LGUI_TexturePackerImporterEditor, Error, TEXT("[LGUI TexturePackerImporter] Load texture file: [%s] failed!"), *jsonFilePath);
		return FReply::Handled();
	}
	if (width != metaTextureWidth || height != metaTextureHeight)
	{
		UE_LOG(LGUI_TexturePackerImporterEditor, Warning, TEXT("[LGUI TexturePackerImporter] Texture size different! Size in json: [%d, %d], size in texture: [%d, %d]"), metaTextureWidth, metaTextureHeight, width, height);
		//return FReply::Handled();
	}

	FString saveFolder;
	if (!GetFolderToSaveSpriteDatas(saveFolder))
	{
		return FReply::Handled();
	}
	float texFullWidthReciprocal = 1.0f / width;
	float texFullHeightReciprocal = 1.0f / height;
	auto framesJObject = jsonObject->GetArrayField("frames");
	bool hasRotated = false;
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
		spriteData.rotated = itemJObject->GetBoolField("rotated");
		spriteData.trimmed = itemJObject->GetBoolField("trimmed");
		auto spriteSourceSizeJObject = itemJObject->GetObjectField("spriteSourceSize");
		spriteData.spriteSourceSize_x = spriteSourceSizeJObject->GetIntegerField("x");
		spriteData.spriteSourceSize_y = spriteSourceSizeJObject->GetIntegerField("y");
		spriteData.spriteSourceSize_w = spriteSourceSizeJObject->GetIntegerField("w");
		spriteData.spriteSourceSize_h = spriteSourceSizeJObject->GetIntegerField("h");
		auto sourceSizeJOjbect = itemJObject->GetObjectField("sourceSize");
		spriteData.sourceSize_w = sourceSizeJOjbect->GetIntegerField("w");
		spriteData.sourceSize_h = sourceSizeJOjbect->GetIntegerField("h");
		if (spriteData.rotated)
		{
			hasRotated = true;
		}

		CreateSpriteData(spriteData, texFullWidthReciprocal, texFullHeightReciprocal, saveFolder);
	}
	if (hasRotated)
	{
		FMessageDialog::Open(EAppMsgType::Ok
			, FText::FromString(FString::Printf(TEXT("Detect rotated sprite, this is not supported!"))));
	}

	int pixelCount = width * height;
	TArray<FColor> atlasTexturePixels;
	atlasTexturePixels.AddUninitialized(pixelCount);
	FMemory::Memcpy((uint8*)atlasTexturePixels.GetData(), rawData.GetData(), pixelCount * 4);

	FString texturePackageName;
	if (IsValid(TargetScriptPtr->atlasTexture))
	{
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
	}
	auto fontTexture = TextureUtils::CreateTextureAsset(TargetScriptPtr->atlasTexture, texturePackageName, width, height, atlasTexturePixels, TextureCompressionSettings::TC_EditorIcon, TextureGroup::TEXTUREGROUP_World);
	TargetScriptPtr->atlasTexture = fontTexture;
	TargetScriptPtr->MarkPackageDirty();
	ULGUIEditorManagerObject::RefreshAllUI();
	return FReply::Handled();
}

bool FLGUITexturePackerImporterDataCustomization::GetFolderToSaveSpriteDatas(FString& OutFolderName)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		if (
			DesktopPlatform->OpenDirectoryDialog(
				FSlateApplication::Get().FindBestParentWindowHandleForDialogs(FSlateApplication::Get().GetGameViewport()),
				TEXT("LGUI will generate a lot of sprite-data assets depend on json file, please choose a folder to save these assets, must inside Content folder"),
				FPaths::ProjectContentDir(),
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
	FString assetName = packageName + "." + SpriteData.name;
	Result = LoadObject<ULGUITexturePackerSpriteData>(NULL, *assetName);
	if (!IsValid(Result))
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 26
		UPackage* package = CreatePackage(*packageName);
#else
		UPackage* package = CreatePackage(NULL, *packageName);
#endif
		package->FullyLoad();
		FString fileName = SpriteData.name;
		Result = NewObject<ULGUITexturePackerSpriteData>(package, ULGUITexturePackerSpriteData::StaticClass(), *fileName, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
	}
	auto& spriteInfo = Result->spriteInfo;
	if (SpriteData.rotated)
	{
		spriteInfo.width = SpriteData.frame_h;
		spriteInfo.height = SpriteData.frame_w;
		spriteInfo.ApplyUV(SpriteData.frame_x, SpriteData.frame_y, SpriteData.frame_h, SpriteData.frame_w, texFullWidthReciprocal, texFullHeightReciprocal);
		spriteInfo.ApplyBorderUV(texFullWidthReciprocal, texFullHeightReciprocal);
	}
	else
	{
		spriteInfo.width = SpriteData.frame_w;
		spriteInfo.height = SpriteData.frame_h;
		spriteInfo.ApplyUV(SpriteData.frame_x, SpriteData.frame_y, SpriteData.frame_w, SpriteData.frame_h, texFullWidthReciprocal, texFullHeightReciprocal);
		spriteInfo.ApplyBorderUV(texFullWidthReciprocal, texFullHeightReciprocal);
	}

	Result->importer = TargetScriptPtr.Get();
	Result->MarkPackageDirty();
	TargetScriptPtr->sprites.Add(Result);
	FAssetRegistryModule::AssetCreated(Result);
	return Result;
}
#undef LOCTEXT_NAMESPACE