// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once
#include "Modules/ModuleInterface.h"

class FLGUI_TexturePackerImporterModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
