// Copyright 2020 Phyronnaz

#include "VoxelEdModeToolkit.h"
#include "VoxelEdMode.h"
#include "VoxelEditorToolsPanel.h"
#include "EditorModeManager.h"

void FVoxelEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	FModeToolkit::Init(InitToolkitHost);
}

FName FVoxelEdModeToolkit::GetToolkitFName() const
{
	return FName("VoxelEdMode");
}

FText FVoxelEdModeToolkit::GetBaseToolkitName() const
{
	return VOXEL_LOCTEXT("VoxelEdMode Tool");
}

class FEdMode* FVoxelEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FEdModeVoxel::EM_Voxel);
}

TSharedPtr<SWidget> FVoxelEdModeToolkit::GetInlineContent() const
{
	return GetPanel().GetWidget();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelEdModeToolkit::GetToolPaletteNames(TArray<FName>& InPaletteName) const
{
	InPaletteName = { STATIC_FNAME("Main") };
}

void FVoxelEdModeToolkit::BuildToolPalette(FName PaletteName, FToolBarBuilder& ToolBarBuilder)
{
	GetPanel().CustomizeToolbar(ToolBarBuilder);
}

void FVoxelEdModeToolkit::OnToolPaletteChanged(FName PaletteName)
{
}

FText FVoxelEdModeToolkit::GetActiveToolDisplayName() const
{
	return {};
}

FText FVoxelEdModeToolkit::GetActiveToolMessage() const
{
	return {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelEditorToolsPanel& FVoxelEdModeToolkit::GetPanel() const
{
	FEdModeVoxel* VoxelEdMode = static_cast<FEdModeVoxel*>(GetEditorMode());
	check(VoxelEdMode);
	return VoxelEdMode->GetPanel();
}