// Copyright 2018 Phyronnaz

#include "VoxelTerrainEdModeCommands.h"
#include "Framework/Commands/Commands.h"
#include "VoxelTerrainEdModeStyle.h"

#define LOCTEXT_NAMESPACE ""

void FVoxelTerrainEdModeCommands::RegisterCommands()
{
	UI_COMMAND(EditMode, "Mode - Edit", "", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(FlattenMode, "Mode - Flatten", "", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(PaintMode, "Mode - Paint", "", EUserInterfaceActionType::RadioButton, FInputChord());
	
	UI_COMMAND(ProjectionTool, "Tool - Projection", "", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(SphereTool, "Tool - Sphere", "", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(BoxTool, "Tool - Box", "", EUserInterfaceActionType::RadioButton, FInputChord());
}


TArray<FVoxelEditorToolOrMode*> FVoxelEditorToolOrMode::GetToolsCommands()
{
	static TArray<FVoxelEditorToolOrMode*> Array =  {new FVoxelProjectionTool(), new FVoxelSphereTool(), new FVoxelBoxTool() };
	return Array;
}

TArray<FVoxelEditorToolOrMode*> FVoxelEditorToolOrMode::GetModesCommands()
{
	static TArray<FVoxelEditorToolOrMode*> Array =  {new FVoxelEditMode(), new FVoxelFlattenMode(), new FVoxelPaintMode() };
	return Array;
}

#undef LOCTEXT_NAMESPACE