// Copyright 2021-present LexLiu. All Rights Reserved.

#pragma once

#include "LGUITexturePackerImporterData.h"
#include "Core/ActorComponent/UIText.h"


#if WITH_EDITOR
void ULGUITexturePackerImporterData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetName();
	}
}
#endif
void ULGUITexturePackerImporterData::AddUISprite(TScriptInterface<IUISpriteRenderableInterface> InUISprite)
{
	renderSpriteArray.AddUnique(InUISprite.GetObject());
}
void ULGUITexturePackerImporterData::RemoveUISprite(TScriptInterface<IUISpriteRenderableInterface> InUISprite)
{
	renderSpriteArray.RemoveSingle(InUISprite.GetObject());
}
bool ULGUITexturePackerImporterData::ContainsSpriteData(ULGUITexturePackerSpriteData* InSpriteData)
{
	return sprites.Contains(InSpriteData);
}
