// Copyright 2021-present LexLiu. All Rights Reserved.

#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FLGUITexturePackerImporterDataCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	struct SpriteDateStruct
	{
		FString name;
		int frame_x, frame_y, frame_w, frame_h;
		int spriteSourceSize_x, spriteSourceSize_y, spriteSourceSize_w, spriteSourceSize_h;
		int sourceSize_w, sourceSize_h;
	};
	TWeakObjectPtr<class ULGUITexturePackerImporterData> TargetScriptPtr;
	FReply OnCreateButtonClicked(
		TSharedRef<IPropertyHandle> jsonFilePropertyHandle
		, TSharedRef<IPropertyHandle> targetFolderPropertyHandle
	);
	void OnPathTextChanged(const FString& InText, TSharedRef<IPropertyHandle> PropertyHandle);
	void OnPathTextCommitted(const FString& InText, TSharedRef<IPropertyHandle> PropertyHandle);
	FText OnGetFontFilePath()const;
	bool GetFolderToSaveSpriteDatas(FString& OutFolderName, const FString& InDefaultFolder);
	class ULGUITexturePackerSpriteData* CreateSpriteData(SpriteDateStruct SpriteData, float texFullWidthReciprocal, float texFullHeightReciprocal, const FString& FolderPath);
};
