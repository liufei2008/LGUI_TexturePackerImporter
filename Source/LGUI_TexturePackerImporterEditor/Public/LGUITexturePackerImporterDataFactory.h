// Copyright 2021-present LexLiu. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "LGUITexturePackerImporterDataFactory.generated.h"

UCLASS()
class ULGUITexturePackerImporterDataFactory : public UFactory
{
	GENERATED_BODY()
public:
	ULGUITexturePackerImporterDataFactory();

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
