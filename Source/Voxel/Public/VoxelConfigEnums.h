// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelConfigEnums.generated.h"

UENUM(BlueprintType)
enum class EVoxelRenderType : uint8
{
	MarchingCubes,
	Cubic
};

UENUM(BlueprintType)
enum class EVoxelNormalConfig : uint8
{
	NoNormal = 0,
	GradientNormal = 1,
	MeshNormal = 2
};

UENUM(BlueprintType)
enum class EVoxelMaterialConfig : uint8
{
	RGB,
	SingleIndex,
	DoubleIndex
};

UENUM(BlueprintType)
enum class EVoxelUVConfig : uint8
{
	GlobalUVs,
	UseRGAsUVs UMETA(DisplayName="Use Red and Green as UVs"),
	PackWorldUpInUVs UMETA(DisplayName="Pack WorldUp in UVs"),
	PerVoxelUVs UMETA(DisplayName="Per Voxel UVs (Cubic only)")
};