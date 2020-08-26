// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Framework/Commands/Commands.h"

class FVoxelDataAssetEditorCommands : public TCommands<FVoxelDataAssetEditorCommands>
{
public:
	/** Constructor */
	FVoxelDataAssetEditorCommands()
		: TCommands<FVoxelDataAssetEditorCommands>
		(
		"VoxelDataAssetEditor", // Context name for icons
		VOXEL_LOCTEXT("Voxel Data Asset Editor"), // Localized context name for displaying
		NAME_None, // Parent
		"VoxelStyle" // Icon Style Set
		)
	{
	}
	
	/** Toggles the preview pane's grid */
	TSharedPtr<FUICommandInfo> TogglePreviewGrid;
	
	/** Toggles the preview pane's background */
	TSharedPtr<FUICommandInfo> TogglePreviewBackground;

	/** Invert data asset */
	TSharedPtr<FUICommandInfo> InvertDataAsset;
	

	/** Initialize commands */
	virtual void RegisterCommands() override
	{
#define LOCTEXT_NAMESPACE "Voxel"
		UI_COMMAND(TogglePreviewGrid, "Grid", "Toggles the preview pane's grid.", EUserInterfaceActionType::ToggleButton, FInputChord());
		UI_COMMAND(TogglePreviewBackground, "Background", "Toggles the preview pane's background.", EUserInterfaceActionType::ToggleButton, FInputChord());
		UI_COMMAND(InvertDataAsset, "Invert", "Invert the asset.", EUserInterfaceActionType::Button, FInputChord());
#undef LOCTEXT_NAMESPACE
	}
};