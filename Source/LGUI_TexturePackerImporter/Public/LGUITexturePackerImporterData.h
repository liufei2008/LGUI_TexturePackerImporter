// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "Core/LGUIFontData_BaseObject.h"
#include "LGUITexturePackerImporterData.generated.h"

class UUISpriteBase;
class ULGUITexturePackerSpriteData;

/** Import asset generated from "TexturePacker". */
UCLASS(BlueprintType)
class LGUI_TEXTUREPACKERIMPORTER_API ULGUITexturePackerImporterData : public UObject
{
	GENERATED_BODY()
private:
	friend class FLGUITexturePackerImporterDataCustomization;

	/**
	 * TexturePacker exported json file path. The texture file should be right aside with it.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FString jsonFilePath;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		FString version;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<ULGUITexturePackerSpriteData*> sprites;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<UUISpriteBase>> renderSpriteArray;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UTexture2D* atlasTexture;
public:
	UTexture2D* GetAtlasTexture()const { return atlasTexture; }
	void AddUISprite(UUISpriteBase* InSprite);
	void RemoveUISprite(UUISpriteBase* InSprite);
	bool ContainsSpriteData(ULGUITexturePackerSpriteData* InSpriteData);
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
#endif
};
