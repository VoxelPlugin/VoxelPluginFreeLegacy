// Copyright 2018 Phyronnaz

#include "VoxelTerrainEdModeDetails.h"
#include "VoxelTerrainEdMode.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "PropertyHandle.h"
#include "PropertyCustomizationHelpers.h"
#include "VoxelTerrainEdModeData.h"
#include "VoxelTerrainEdModeCommands.h"
#include "VoxelTerrainEdModeStyle.h"

#define LOCTEXT_NAMESPACE "FVoxelTerrainEdModeDetails"

TSharedRef<IDetailCustomization> FVoxelTerrainEdModeDetails::MakeInstance()
{
	return MakeShareable(new FVoxelTerrainEdModeDetails);
}

void FVoxelTerrainEdModeDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	CommandList = GetEditorMode()->GetUICommandList();
}

void FVoxelTerrainEdModeDetails::UpdateSculptInfo()
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = GetEditorMode();
}

FVoxelTerrainEdMode * FVoxelTerrainEdModeDetails::GetEditorMode()
{
	return (FVoxelTerrainEdMode*)GLevelEditorModeTools().GetActiveMode(FVoxelTerrainEdMode::EM_VoxelTerrainEdModeId);
}

bool FVoxelTerrainEdModeDetails::IsToolActive(FName ToolName)
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = GetEditorMode();
	
	return false;
}

bool FVoxelTerrainEdModeDetails::IsToolModeActive(FName ToolModeName)
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = GetEditorMode();
	
	return false;
}

#undef LOCTEXT_NAMESPACE