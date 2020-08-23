// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelWhiteNoiseNodes.generated.h"

// 2D White Noise
UCLASS(DisplayName = "2D White Noise", Category = "Noise|White Noise")
class VOXELGRAPH_API UVoxelNode_2DWhiteNoise : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_2DWhiteNoise();
};

// 3D White Noise
UCLASS(DisplayName = "3D White Noise", Category = "Noise|White Noise")
class VOXELGRAPH_API UVoxelNode_3DWhiteNoise : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_3DWhiteNoise();
};