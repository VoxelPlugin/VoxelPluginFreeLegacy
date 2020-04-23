// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;

namespace FVoxelUtilities
{
	VOXEL_API bool IsMaterialTessellated(UMaterialInterface* Material);

	// NumIndices: else chunks get merged with different number of tex coordinates
	VOXEL_API UMaterialInterface* GetDefaultMaterial(bool bTessellation, int32 NumIndices);
}
