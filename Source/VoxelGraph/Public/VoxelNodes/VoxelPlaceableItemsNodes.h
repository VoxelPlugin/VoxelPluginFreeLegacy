// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelPlaceableItemsNodes.generated.h"

// Return 0 when outside a worm, and the distance to the closest wall when inside
// X Y Z inputs must be relatively close to the real XYZ, else the wrong worms will be used
UCLASS(DisplayName = "Perlin Worm Distance", Category = "Placeable Items")
class VOXELGRAPH_API UVoxelNode_PerlinWormDistance : public UVoxelNodeWithContext
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_PerlinWormDistance();
};