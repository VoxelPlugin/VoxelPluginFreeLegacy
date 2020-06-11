// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerTool.h"

class FVoxelToolManagerTool_Flatten : public FVoxelToolManagerTool
{
public:
	const FVoxelToolManager_FlattenSettings& FlattenSettings;
	
	explicit FVoxelToolManagerTool_Flatten(const UVoxelToolManager& ToolManager);
	
	//~ Begin FVoxelToolManagerTool Interface
	virtual void Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData) override;
	//~ End FVoxelToolManagerTool Interface

private:
	FVector LastClickFlattenPosition = FVector::ZeroVector;
	FVector LastClickFlattenNormal = FVector::UpVector;
};