// Copyright 2020 Phyronnaz

#include "VoxelEdModeToolkit.h"
#include "VoxelEdMode.h"
#include "VoxelEditorToolsPanel.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "Voxel"

void FVoxelEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	FEdModeVoxel* VoxelEdMode = static_cast<FEdModeVoxel*>(GetEditorMode());
	check(VoxelEdMode);

	ToolkitWidget = VoxelEdMode->GetPanel().GetWidget();

	FModeToolkit::Init(InitToolkitHost);
}

FName FVoxelEdModeToolkit::GetToolkitFName() const
{
	return FName("VoxelEdMode");
}

FText FVoxelEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("VoxelEdModeToolkit", "DisplayName", "VoxelEdMode Tool");
}

class FEdMode* FVoxelEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FEdModeVoxel::EM_Voxel);
}

#undef LOCTEXT_NAMESPACE