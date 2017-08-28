// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IPluginManager.h"
#include "Brushes/SlateImageBrush.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) FSlateBorderBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelativePath, ...) FSlateFontInfo(RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)
#define OTF_FONT(RelativePath, ...) FSlateFontInfo(RootToContentDir(RelativePath, TEXT(".otf")), __VA_ARGS__)


/**
 * Implements the visual style of the text asset editor UI.
 */
class FVoxelEditorStyle
	: public FSlateStyleSet
{
public:

	/** Default constructor. */
	 FVoxelEditorStyle()
		 : FSlateStyleSet("VoxelEditorStyle")
	 {
		const FVector2D Icon16x16(16.0f, 16.0f);
		const FVector2D Icon20x20(20.0f, 20.0f);
		const FVector2D Icon40x40(40.0f, 40.0f);

		const FString BaseDir = IPluginManager::Get().FindPlugin("Voxel")->GetBaseDir();
		SetContentRoot(BaseDir / TEXT("Content"));

		// set new styles here, for example...
		//Set("VoxelEditor.FancyButton", new IMAGE_BRUSH("icon_forward_40x", Icon40x40));
		
		FSlateStyleRegistry::RegisterSlateStyle(*this);
	 }

	 /** Destructor. */
	 ~FVoxelEditorStyle()
	 {
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	 }
};


#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT
