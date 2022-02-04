// Copyright 2021-present LexLiu. All Rights Reserved.

#pragma once

#include "LGUITexturePackerSpriteData.h"
#include "LGUITexturePackerImporterData.h"
#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/Actor/LGUIManagerActor.h"

UTexture2D* ULGUITexturePackerSpriteData::GetAtlasTexture()
{
	if (IsValid(importer))
	{
		return importer->GetAtlasTexture();
	}
	else
	{
		return nullptr;
	}
}
FLGUISpriteInfo ULGUITexturePackerSpriteData::GetSpriteInfo()
{
	return spriteInfo;
}
bool ULGUITexturePackerSpriteData::IsIndividual()const
{
	return false;
}
void ULGUITexturePackerSpriteData::AddUISprite(UUISpriteBase* InUISprite)
{
	if (IsValid(importer))
	{
		importer->AddUISprite(InUISprite);
	}
}
void ULGUITexturePackerSpriteData::RemoveUISprite(UUISpriteBase* InUISprite)
{
	if (IsValid(importer))
	{
		importer->RemoveUISprite(InUISprite);
	}
}
#if WITH_EDITOR
void ULGUITexturePackerSpriteData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		if (
			PropertyName == GET_MEMBER_NAME_CHECKED(FLGUISpriteInfo, borderLeft) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FLGUISpriteInfo, borderRight) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FLGUISpriteInfo, borderTop) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FLGUISpriteInfo, borderBottom)
			)
		{
			//sprite data, apply border
			if (IsValid(GetAtlasTexture()))
			{
				float atlasTextureSizeXInv = 1.0f / GetAtlasTexture()->GetSizeX();
				float atlasTextureSizeYInv = 1.0f / GetAtlasTexture()->GetSizeY();
				spriteInfo.ApplyBorderUV(atlasTextureSizeXInv, atlasTextureSizeYInv);
			}
		}
		ULGUIEditorManagerObject::RefreshAllUI();
	}
}
#endif
