// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUITexturePackerSpriteDataCustomization.h"
#include "DesktopPlatformModule.h"
#include "Core/LGUISettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Core/LGUIAtlasData.h"
#include "LGUI_TexturePackerImporterEditorPrivatePCH.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Sound/SoundCue.h"

#define LOCTEXT_NAMESPACE "LGUITexturePackerSpriteDataCustomization"

TSharedRef<IDetailCustomization> FLGUITexturePackerSpriteDataCustomization::MakeInstance()
{
	return MakeShareable(new FLGUITexturePackerSpriteDataCustomization);
}

void FLGUITexturePackerSpriteDataCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULGUITexturePackerSpriteData>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUI_TexturePackerImporterEditor, Log, TEXT("Get TargetScript is null"));
		return;
	}
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULGUITexturePackerSpriteData, spriteInfo));
	IDetailCategoryBuilder& lguiCategory = DetailBuilder.EditCategory("LGUI");
	if (!IsValid(TargetScriptPtr->importer))
	{
		lguiCategory.AddCustomRow(LOCTEXT("ImporterError", "ImporterError"))
		.WholeRowContent()
		[
			SNew(SBox)
			.HeightOverride(30)
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Not valid! Missing TexturePacker Importer!"))
				.AutoWrapText(true)
				.ColorAndOpacity(FLinearColor::Red)
			]
		];
	}
	else
	{
		if (!IsValid(TargetScriptPtr->importer->GetAtlasTexture()))
		{
			lguiCategory.AddCustomRow(LOCTEXT("TextureError", "TextureError"))
			.WholeRowContent()
			[
				SNew(SBox)
				.HeightOverride(30)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Not valid! Missing atlas texture! Please check TexturePacker Importer's texture."))
					.AutoWrapText(true)
					.ColorAndOpacity(FLinearColor::Red)
				]
			];
		}

		if (!TargetScriptPtr->importer->ContainsSpriteData(TargetScriptPtr.Get()))
		{
			lguiCategory.AddCustomRow(LOCTEXT("Error", "Error"))
			.WholeRowContent()
			[
				SNew(SBox)
				.HeightOverride(30)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Not valid! This sprite not belongs to registerred TexturePacker Importer!"))
					.AutoWrapText(true)
					.ColorAndOpacity(FLinearColor::Red)
				]
			];
		}
	}

	lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUITexturePackerSpriteData, spriteInfo.width));
	lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUITexturePackerSpriteData, spriteInfo.height));
	
	auto spriteTexture = TargetScriptPtr->GetAtlasTexture();
	if (IsValid(spriteTexture))
	{
		IDetailCategoryBuilder& borderEditorCategory = DetailBuilder.EditCategory("BorderEditor");
		spriteSlateBrush = TSharedPtr<FSlateBrush>(new FSlateBrush);
		spriteSlateBrush->SetResourceObject(spriteTexture);
		auto spriteInfo = TargetScriptPtr->GetSpriteInfo();
		spriteSlateBrush->SetUVRegion(FBox2D(FVector2D(spriteInfo.uv0X, spriteInfo.uv3Y), FVector2D(spriteInfo.uv3X, spriteInfo.uv0Y)));
		borderEditorCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUITexturePackerSpriteData, spriteInfo.borderLeft));
		borderEditorCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUITexturePackerSpriteData, spriteInfo.borderRight));
		borderEditorCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUITexturePackerSpriteData, spriteInfo.borderTop));
		borderEditorCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUITexturePackerSpriteData, spriteInfo.borderBottom));
		borderEditorCategory.AddCustomRow(LOCTEXT("BorderEditor", "BorderEditor"))
		.WholeRowContent()
		[
			SNew(SBorder)
			[
				SAssignNew(ImageBox, SBox)
				//.MinDesiredHeight(this, &FLGUITexturePackerSpriteDataCustomization::GetMinDesiredHeight, &DetailBuilder)
				.HeightOverride(this, &FLGUITexturePackerSpriteDataCustomization::GetMinDesiredHeight, &DetailBuilder)
				//.MinDesiredHeight(4096)
				//.MinDesiredWidth(4096)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.HAlign(EHorizontalAlignment::HAlign_Center)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(SOverlay)
							//image background
							+SOverlay::Slot()
							[
								SNew(SImage)
								.Image(FEditorStyle::GetBrush("Checkerboard"))
								.ColorAndOpacity(FSlateColor(FLinearColor(0.15f, 0.15f, 0.15f)))
							]
							//image display
							+SOverlay::Slot()
							[
								SNew(SBox)
								.WidthOverride(this, &FLGUITexturePackerSpriteDataCustomization::GetImageWidth)
								.HeightOverride(this, &FLGUITexturePackerSpriteDataCustomization::GetImageHeight)
								[
									SNew(SImage)
									.Image(spriteSlateBrush.Get())
								]
							]
							//left splitter
							+SOverlay::Slot()
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.HAlign(EHorizontalAlignment::HAlign_Left)
								[
									SNew(SBox)
									.WidthOverride(this, &FLGUITexturePackerSpriteDataCustomization::GetBorderLeftSize)
									[
										SNew(SBox)
										.HAlign(EHorizontalAlignment::HAlign_Right)
										.WidthOverride(1)
										[
											SNew(SImage)
											.Image(FEditorStyle::GetBrush("PropertyEditor.VerticalDottedLine"))
										]
									]
								]
							]
							//right splitter
							+SOverlay::Slot()
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.HAlign(EHorizontalAlignment::HAlign_Right)
								[
									SNew(SBox)
									.WidthOverride(this, &FLGUITexturePackerSpriteDataCustomization::GetBorderRightSize)
									[
										SNew(SBox)
										.HAlign(EHorizontalAlignment::HAlign_Left)
										.WidthOverride(1)
										[
											SNew(SImage)
											.Image(FEditorStyle::GetBrush("PropertyEditor.VerticalDottedLine"))
										]
									]
								]
							]
							//top splitter
							+SOverlay::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.VAlign(EVerticalAlignment::VAlign_Top)
								[
									SNew(SBox)
									.HeightOverride(this, &FLGUITexturePackerSpriteDataCustomization::GetBorderTopSize)
									[
										SNew(SBox)
										.VAlign(EVerticalAlignment::VAlign_Bottom)
										.HeightOverride(1)
										[
											SNew(SImage)
											.Image(FEditorStyle::GetBrush("PropertyEditor.HorizontalDottedLine"))
										]
									]
								]
							]
							//bottom splitter
							+SOverlay::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.VAlign(EVerticalAlignment::VAlign_Bottom)
								[
									SNew(SBox)
									.HeightOverride(this, &FLGUITexturePackerSpriteDataCustomization::GetBorderBottomSize)
									[
										SNew(SBox)
										.VAlign(EVerticalAlignment::VAlign_Top)
										.HeightOverride(1)
										[
											SNew(SImage)
											.Image(FEditorStyle::GetBrush("PropertyEditor.HorizontalDottedLine"))
										]
									]
								]
							]
						]
					]
				]
			]
		];
	}
}

FOptionalSize FLGUITexturePackerSpriteDataCustomization::GetImageWidth()const
{
	float imageAspect = (float)(TargetScriptPtr->GetSpriteInfo().width) / TargetScriptPtr->GetSpriteInfo().height;
	auto imageBoxSize = ImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return imageBoxSize.X;
	}
	else
	{
		return imageBoxSize.Y * imageAspect;
	}
}
FOptionalSize FLGUITexturePackerSpriteDataCustomization::GetImageHeight()const
{
	float imageAspect = (float)(TargetScriptPtr->GetSpriteInfo().width) / TargetScriptPtr->GetSpriteInfo().height;
	auto imageBoxSize = ImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return imageBoxSize.X / imageAspect;
	}
	else
	{
		return imageBoxSize.Y;
	}
}
FOptionalSize FLGUITexturePackerSpriteDataCustomization::GetMinDesiredHeight(IDetailLayoutBuilder* DetailBuilder)const
{
	return DetailBuilder->GetDetailsView()->GetCachedGeometry().GetLocalSize().Y - 300;
}
FOptionalSize FLGUITexturePackerSpriteDataCustomization::GetBorderLeftSize()const
{
	if (TargetScriptPtr.Get() == nullptr)return 0;
	float imageAspect = (float)(TargetScriptPtr->GetSpriteInfo().width) / TargetScriptPtr->GetSpriteInfo().height;
	auto imageBoxSize = ImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return TargetScriptPtr->spriteInfo.borderLeft * imageBoxSize.X / TargetScriptPtr->GetSpriteInfo().width;
	}
	else
	{
		return TargetScriptPtr->spriteInfo.borderLeft * imageBoxSize.Y / TargetScriptPtr->GetSpriteInfo().height;
	}
}
FOptionalSize FLGUITexturePackerSpriteDataCustomization::GetBorderRightSize()const
{
	if (TargetScriptPtr.Get() == nullptr)return 0;
	float imageAspect = (float)(TargetScriptPtr->GetSpriteInfo().width) / TargetScriptPtr->GetSpriteInfo().height;
	auto imageBoxSize = ImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return TargetScriptPtr->spriteInfo.borderRight * imageBoxSize.X / TargetScriptPtr->GetSpriteInfo().width;
	}
	else
	{
		return TargetScriptPtr->spriteInfo.borderRight * imageBoxSize.Y / TargetScriptPtr->GetSpriteInfo().height;
	}
}
FOptionalSize FLGUITexturePackerSpriteDataCustomization::GetBorderTopSize()const
{
	if (TargetScriptPtr.Get() == nullptr)return 0;
	float imageAspect = (float)(TargetScriptPtr->GetSpriteInfo().width) / TargetScriptPtr->GetSpriteInfo().height;
	auto imageBoxSize = ImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return TargetScriptPtr->spriteInfo.borderTop * imageBoxSize.X / TargetScriptPtr->GetSpriteInfo().width;
	}
	else
	{
		return TargetScriptPtr->spriteInfo.borderTop * imageBoxSize.Y / TargetScriptPtr->GetSpriteInfo().height;
	}
}
FOptionalSize FLGUITexturePackerSpriteDataCustomization::GetBorderBottomSize()const
{
	if (TargetScriptPtr.Get() == nullptr)return 0;
	float imageAspect = (float)(TargetScriptPtr->GetSpriteInfo().width) / TargetScriptPtr->GetSpriteInfo().height;
	auto imageBoxSize = ImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return TargetScriptPtr->spriteInfo.borderBottom * imageBoxSize.X / TargetScriptPtr->GetSpriteInfo().width;
	}
	else
	{
		return TargetScriptPtr->spriteInfo.borderBottom * imageBoxSize.Y / TargetScriptPtr->GetSpriteInfo().height;
	}
}

#undef LOCTEXT_NAMESPACE