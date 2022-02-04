// Copyright 2021-present LexLiu. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LGUI_TexturePackerImporterEditor, Log, All)

class FLGUI_TexturePackerImporterEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	TSharedPtr<class FAssetTypeActions_Base> TPDataTypeAction;
};