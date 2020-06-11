// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerTool.h"

class FVoxelToolManagerTool_Revert : public FVoxelToolManagerTool
{
public:
	const FVoxelToolManager_RevertSettings& RevertSettings;
	
	explicit FVoxelToolManagerTool_Revert(const UVoxelToolManager& ToolManager);
	
	//~ Begin FVoxelToolManagerTool Interface
	virtual void Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData) override;
	//~ End FVoxelToolManagerTool Interface
};