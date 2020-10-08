// Copyright 2020 Phyronnaz

#include "VoxelToolsCommands.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "VoxelToolsCommands"

FVoxelToolsCommands::FVoxelToolsCommands()
	: TCommands<FVoxelToolsCommands>(
		"VoxelTools",
		NSLOCTEXT("Contexts", "VoxelTools", "Voxel Tools"),
		NAME_None,
		"VoxelStyle")
{
}

void FVoxelToolsCommands::RegisterCommands()
{
	UI_COMMAND(FlattenTool, "Flatten", "Progressively flatten an area", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::Five));
	UI_COMMAND(LevelTool, "Level", "Quickly block out flat surfaces. Stamps a cylinder.", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::Six));
	UI_COMMAND(MeshTool, "Mesh", "Smoothly or progressively stamps any mesh", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::Three));
	UI_COMMAND(RevertTool, "Revert", "Reverts edits to the generator (shift) or a specific history position", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::Eight));
	UI_COMMAND(SmoothTool, "Smooth", "Very useful with voxels. Shift makes it slower.", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::Two));
	UI_COMMAND(SphereTool, "Sphere", "Add/remove spheres", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::Four));
	UI_COMMAND(SurfaceTool, "Surface", "Main sculpt tool to smoothly edit the voxels, optionally with masks", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::One));
	UI_COMMAND(TrimTool, "Trim", "Quickly trim/flatten an area. Stamps/remove a half sphere based on the average position/normal under the cursor.", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::Seven));
	
	UI_COMMAND(IncreaseBrushSize, "Increase Brush Size", "Press this key to increase brush size.", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::RightBracket));
	UI_COMMAND(DecreaseBrushSize, "Decrease Brush Size", "Press this key to decrease brush size.", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::LeftBracket));
	UI_COMMAND(IncreaseBrushFalloff, "Increase Brush Falloff", "Press this key to increase brush falloff.", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(DecreaseBrushFalloff, "Decrease Brush Falloff", "Press this key to decrease brush falloff.", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(IncreaseBrushStrength, "Increase Brush Strength", "Press this key to increase brush strength.", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::Period));
	UI_COMMAND(DecreaseBrushStrength, "Decrease Brush Strength", "Press this key to decrease brush strength.", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::Comma));
}

#undef LOCTEXT_NAMESPACE