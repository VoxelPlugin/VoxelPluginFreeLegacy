// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelConfigEnums.generated.h"

UENUM(BlueprintType)
enum class EVoxelRenderType : uint8
{
	MarchingCubes,
	Cubic,
	SurfaceNets
};

UENUM(BlueprintType)
enum class EVoxelNormalConfig : uint8
{
	NoNormal = 0,
	// Use the density field gradient as normal. Might have glitches on hard corners which can be quite visible when using triplanar projection
	GradientNormal = 1,
	// Compute the normal from the mesh faces. This will have glitches on chunks borders, Gradient Normal are preferred
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
	GlobalUVs			UMETA(DisplayName = "Global UVs"),
	PackWorldUpInUVs	UMETA(DisplayName = "Pack WorldUp in UVs"),
	// In Cubic, per Voxel. In others, per chunk
	PerVoxelUVs			UMETA(DisplayName = "Per Voxel/Chunk UVs"),
	Max					UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EVoxelRGBA : uint8
{
	R,
	G,
	B,
	A
};

UENUM(BlueprintType)
enum class EVoxelSpawnerActorSpawnType : uint8
{
	// Spawn all spawner actors
	All,
	// Spawn only floating spawner actors
	OnlyFloating
};

UENUM(BlueprintType)
enum class EVoxelSamplerMode : uint8
{
	// Clamp the coordinates
	Clamp,
	// Tile the coordinates
	Tile
};

enum class EVoxelPlayType
{
	Game,
	Preview
};