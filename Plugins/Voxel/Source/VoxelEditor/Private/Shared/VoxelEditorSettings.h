// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateColor.h"
#include "UObject/ObjectMacros.h"

#include "VoxelEditorSettings.generated.h"


UCLASS(config=Editor)
class VOXELEDITOR_API UVoxelEditorSettings
	: public UObject
{
	GENERATED_BODY()

public:

	/** Color of the Voxel editor's background. */
	UPROPERTY(config, EditAnywhere, Category=Appearance)
	FSlateColor BackgroundColor;

	/** Color of the Voxel editor's text. */
	UPROPERTY(config, EditAnywhere, Category=Appearance)
	FSlateColor ForegroundColor;

	/** The font to use in the Voxel editor window. */
	UPROPERTY(config, EditAnywhere, Category=Appearance)
	FSlateFontInfo Font;

	/** The margin around the Voxel editor window (in pixels). */
	UPROPERTY(config, EditAnywhere, Category=Appearance)
	float Margin;

public:

	/** Default constructor. */
	UVoxelEditorSettings();
};
