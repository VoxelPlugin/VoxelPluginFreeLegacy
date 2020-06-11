// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerTool.h"

class FVoxelToolManagerTool_Trim : public FVoxelToolManagerTool
{
public:
	const FVoxelToolManager_TrimSettings& TrimSettings;
	
	explicit FVoxelToolManagerTool_Trim(const UVoxelToolManager& ToolManager);
	
	//~ Begin FVoxelToolManagerTool Interface
	virtual void Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData) override;
	//~ End FVoxelToolManagerTool Interface
};