// Copyright 2021-present LexLiu. All Rights Reserved.

#include "LGUI_TexturePackerImporterModule.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FLGUIModule"

void FLGUI_TexturePackerImporterModule::StartupModule()
{
	
}

void FLGUI_TexturePackerImporterModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLGUI_TexturePackerImporterModule, LGUI_TexturePackerImporter)