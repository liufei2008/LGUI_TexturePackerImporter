// Copyright 2021-present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/LGUISpriteData_BaseObject.h"
#include "LGUITexturePackerSpriteData.generated.h"

class ULGUITexturePackerImporterData;

/** Render "TexturePacker" generated asset as LGUISpriteData. */
UCLASS(BlueprintType)
class LGUI_TEXTUREPACKERIMPORTER_API ULGUITexturePackerSpriteData : public ULGUISpriteData_BaseObject
{
	GENERATED_BODY()
private:
	friend class FLGUITexturePackerSpriteDataCustomization;
	friend class FLGUITexturePackerImporterDataCustomization;

	/** Information needed for render this sprite */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FLGUISpriteInfo spriteInfo;
	UPROPERTY(VisibleAnywhere, Category = "LGUI", meta=(DisplayThumbnail=false))
		ULGUITexturePackerImporterData* importer;
public:
	//Begin ULGUISpriteData_BaseObject interface
	virtual UTexture2D* GetAtlasTexture()override;
	virtual const FLGUISpriteInfo& GetSpriteInfo()override;
	virtual bool IsIndividual()const override;
	virtual void AddUISprite(TScriptInterface<IUISpriteRenderableInterface> InUISprite) override;
	virtual void RemoveUISprite(TScriptInterface<IUISpriteRenderableInterface> InUISprite) override;
	//End ULGUISpriteData_BaseObject interface

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
#endif
};
