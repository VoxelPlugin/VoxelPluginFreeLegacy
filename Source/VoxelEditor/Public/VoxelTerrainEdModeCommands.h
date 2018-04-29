// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Commands.h"
#include "VoxelTerrainEdModeStyle.h"

#define LOCTEXT_NAMESPACE ""

/**
* Unreal landscape editor actions
*/
class FVoxelTerrainEdModeCommands : public TCommands<FVoxelTerrainEdModeCommands>
{

public:
	FVoxelTerrainEdModeCommands() : TCommands<FVoxelTerrainEdModeCommands>
		(
			"VoxelTerrainEdMode", // Context name for fast lookup
			NSLOCTEXT("Contexts", "VoxelTerrainEdMode", "Voxel Terrain Editor Mode"), // Localized context name for displaying
			NAME_None, // Parent
			FVoxelTerrainEdModeStyle::GetStyleSetName() // Icon Style Set
		)
	{
	}


	virtual void RegisterCommands() override;

public:
	// Modes
	TSharedPtr<FUICommandInfo> EditMode;
	TSharedPtr<FUICommandInfo> FlattenMode;
	TSharedPtr<FUICommandInfo> PaintMode;

	// Tools
	TSharedPtr<FUICommandInfo> ProjectionTool;
	TSharedPtr<FUICommandInfo> SphereTool;
	TSharedPtr<FUICommandInfo> BoxTool;
};

class FVoxelEditorToolOrMode
{
public:
	static TArray<FVoxelEditorToolOrMode*> GetToolsCommands();
	static TArray<FVoxelEditorToolOrMode*> GetModesCommands();

	virtual FName GetName() const = 0;
	virtual TSharedPtr<FUICommandInfo> GetCommand() const = 0;
	virtual FText GetToolTip() const = 0;
	virtual FText GetLabel() const = 0;
	virtual FName GetIconName() const = 0;
};

class FVoxelEditMode : public FVoxelEditorToolOrMode
{
	virtual FName GetName() const override { return FName(TEXT("EditMode")); }
	virtual TSharedPtr<FUICommandInfo> GetCommand() const override { return FVoxelTerrainEdModeCommands::Get().EditMode; }
	virtual FText GetToolTip() const override { return LOCTEXT("ToolTip", "ToolTip"); }
	virtual FText GetLabel() const override { return LOCTEXT("Edit", "Edit"); }
	virtual FName GetIconName() const override { return TEXT("Plugins.Mode.Edit"); }
};

class FVoxelFlattenMode : public FVoxelEditorToolOrMode
{
	virtual FName GetName() const override { return FName(TEXT("FlattenMode")); }
	virtual TSharedPtr<FUICommandInfo> GetCommand() const override { return FVoxelTerrainEdModeCommands::Get().FlattenMode; }
	virtual FText GetToolTip() const override { return LOCTEXT("ToolTip", "ToolTip"); }
	virtual FText GetLabel() const override { return LOCTEXT("Flatten", "Flatten"); }
	virtual FName GetIconName() const override { return TEXT("Plugins.Mode.Flatten"); }
};

class FVoxelPaintMode : public FVoxelEditorToolOrMode
{
	virtual FName GetName() const override { return FName(TEXT("PaintMode")); }
	virtual TSharedPtr<FUICommandInfo> GetCommand() const override { return FVoxelTerrainEdModeCommands::Get().PaintMode; }
	virtual FText GetToolTip() const override { return LOCTEXT("ToolTip", "ToolTip"); }
	virtual FText GetLabel() const override { return LOCTEXT("Paint", "Paint"); }
	virtual FName GetIconName() const override { return TEXT("Plugins.Mode.Paint"); }
};

class FVoxelProjectionTool : public FVoxelEditorToolOrMode
{
	virtual FName GetName() const override { return FName(TEXT("ProjectionTool")); }
	virtual TSharedPtr<FUICommandInfo> GetCommand() const override { return FVoxelTerrainEdModeCommands::Get().ProjectionTool; }
	virtual FText GetToolTip() const override { return LOCTEXT("ToolTip", "ToolTip"); }
	virtual FText GetLabel() const override { return LOCTEXT("Projection", "Projection"); }
	virtual FName GetIconName() const override { return TEXT("Plugins.Tool.Projection"); }
};

class FVoxelSphereTool : public FVoxelEditorToolOrMode
{
	virtual FName GetName() const override { return FName(TEXT("SphereTool")); }
	virtual TSharedPtr<FUICommandInfo> GetCommand() const override { return FVoxelTerrainEdModeCommands::Get().SphereTool; }
	virtual FText GetToolTip() const override { return LOCTEXT("ToolTip", "ToolTip"); }
	virtual FText GetLabel() const override { return LOCTEXT("Sphere", "Sphere"); }
	virtual FName GetIconName() const override { return TEXT("Plugins.Tool.Sphere"); }
};

class FVoxelBoxTool : public FVoxelEditorToolOrMode
{
	virtual FName GetName() const override { return FName(TEXT("BoxTool")); }
	virtual TSharedPtr<FUICommandInfo> GetCommand() const override { return FVoxelTerrainEdModeCommands::Get().BoxTool; }
	virtual FText GetToolTip() const override { return LOCTEXT("ToolTip", "ToolTip"); }
	virtual FText GetLabel() const override { return LOCTEXT("Box", "Box"); }
	virtual FName GetIconName() const override { return TEXT("Plugins.Tool.Box"); }
};

#undef LOCTEXT_NAMESPACE