// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "LGUITexturePackerImporterData.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/Actor/LGUIManagerActor.h"


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
void ULGUITexturePackerImporterData::AddUISprite(UUISpriteBase* InSprite)
{
	renderSpriteArray.Add(InSprite);
}
void ULGUITexturePackerImporterData::RemoveUISprite(UUISpriteBase* InSprite)
{
	renderSpriteArray.Remove(InSprite);
}
bool ULGUITexturePackerImporterData::ContainsSpriteData(ULGUITexturePackerSpriteData* InSpriteData)
{
	return sprites.Contains(InSpriteData);
}
