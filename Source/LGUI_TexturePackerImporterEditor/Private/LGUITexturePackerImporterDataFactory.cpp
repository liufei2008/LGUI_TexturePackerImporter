// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUITexturePackerImporterDataFactory.h"
#include "LGUI_TexturePackerImporterEditorPrivatePCH.h"

#define LOCTEXT_NAMESPACE "LGUITexturePackerImporterDataFactory"


ULGUITexturePackerImporterDataFactory::ULGUITexturePackerImporterDataFactory()
{
	SupportedClass = ULGUITexturePackerImporterData::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}
UObject* ULGUITexturePackerImporterDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<ULGUITexturePackerImporterData>(InParent, Class, Name, Flags | RF_Transactional);
}

#undef LOCTEXT_NAMESPACE
