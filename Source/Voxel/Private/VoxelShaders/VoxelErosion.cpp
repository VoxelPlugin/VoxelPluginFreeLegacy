// Copyright 2020 Phyronnaz

#include "VoxelShaders/VoxelErosion.h"
#include "VoxelMathUtilities.h"
#include "VoxelMessages.h"

#include "Engine/Texture2D.h"
#include "Logging/MessageLog.h"
#include "Logging/TokenizedMessage.h"

void UVoxelErosion::Initialize()
{
	VOXEL_PRO_ONLY_VOID();
}

bool UVoxelErosion::IsInitialized() const
{
	VOXEL_PRO_ONLY();
}

void UVoxelErosion::Step(int32 Count)
{
	VOXEL_PRO_ONLY_VOID();
}

FVoxelFloatTexture UVoxelErosion::GetTerrainHeightTexture()
{
	VOXEL_PRO_ONLY();
}


FVoxelFloatTexture UVoxelErosion::GetWaterHeightTexture()
{
	VOXEL_PRO_ONLY();
}


FVoxelFloatTexture UVoxelErosion::GetSedimentTexture()
{
	VOXEL_PRO_ONLY();
}

