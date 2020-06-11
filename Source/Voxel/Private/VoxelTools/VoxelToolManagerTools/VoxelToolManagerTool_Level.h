// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerTool.h"

class FVoxelToolManagerTool_Level : public FVoxelToolManagerTool
{
public:
	const FVoxelToolManager_LevelSettings& LevelSettings;
	
	explicit FVoxelToolManagerTool_Level(const UVoxelToolManager& ToolManager);
	
	//~ Begin FVoxelToolManagerTool Interface
	virtual void Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData) override;
	//~ End FVoxelToolManagerTool Interface
};