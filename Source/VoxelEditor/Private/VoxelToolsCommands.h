// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

/**
 * Unreal landscape editor actions
 */
class FVoxelToolsCommands : public TCommands<FVoxelToolsCommands>
{

public:
	FVoxelToolsCommands();
	
	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> FlattenTool;
	TSharedPtr<FUICommandInfo> LevelTool;
	TSharedPtr<FUICommandInfo> MeshTool;
	TSharedPtr<FUICommandInfo> RevertTool;
	TSharedPtr<FUICommandInfo> SmoothTool;
	TSharedPtr<FUICommandInfo> SphereTool;
	TSharedPtr<FUICommandInfo> SurfaceTool;
	TSharedPtr<FUICommandInfo> TrimTool;

	TSharedPtr<FUICommandInfo> IncreaseBrushSize;
	TSharedPtr<FUICommandInfo> DecreaseBrushSize;
	TSharedPtr<FUICommandInfo> IncreaseBrushFalloff;
	TSharedPtr<FUICommandInfo> DecreaseBrushFalloff;
	TSharedPtr<FUICommandInfo> IncreaseBrushStrength;
	TSharedPtr<FUICommandInfo> DecreaseBrushStrength;
};