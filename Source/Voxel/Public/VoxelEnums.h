// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.generated.h"

UENUM(BlueprintType)
enum class EVoxelRenderType : uint8
{
	MarchingCubes,
	Cubic,
	// Surface nets only work well at LOD 0. They will have holes between higher LODs, and the material won't be picked correctly.
	SurfaceNets
};

UENUM(BlueprintType)
enum class EVoxelNormalConfig : uint8
{
	NoNormal,
	// Use the density field gradient as normal. Might have glitches on hard corners which can be quite visible when using triplanar projection
	GradientNormal,
	// Each vertex will be duplicated & its normal set to the face normal
	// This will disable vertex translating on transitions between LODs as the normals are not the same anymore
	// This will not create any holes, but the transitions might look slightly worse (tiny vertical faces)
	FlatNormal,
	// Compute the normal from the mesh faces. This will have glitches on chunks borders, Gradient Normal are preferred
	MeshNormal
};

UENUM(BlueprintType)
enum class EVoxelMaterialConfig : uint8
{
	RGB,
	SingleIndex,
	DoubleIndex_DEPRECATED UMETA(Hidden),
	MultiIndex
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

UENUM(BlueprintType)
enum class EVoxelDataType : uint8
{
	Values,
	Materials
};

UENUM(BlueprintType)
enum class EVoxelRGBHardness : uint8
{
	// Interpret the material as 4 way blend, and use MaterialsHardness
	FourWayBlend,
	// Interpret the material as 5 way blend, and use MaterialsHardness
	FiveWayBlend,
	// Use the Red channel as hardness
	R,
	// Use the Green channel as hardness
	G,
	// Use the Blue channel as hardness
	B,
	// Use the Alpha channel as hardness
	A,
	// Use the U0 channel as hardness
	U0,
	// Use the U1 channel as hardness
	U1,
	// Use the V0 channel as hardness
	V0,
	// Use the V1 channel as hardness
	V1
};

UENUM(BlueprintType)
enum class EVoxelFalloff : uint8
{
	Linear,
	Smooth,
	Spherical,
	Tip
};

UENUM(BlueprintType)
enum class EVoxelComputeDevice : uint8
{
	CPU,
	GPU
};

UENUM(BlueprintType)
enum class EVoxelAxis : uint8
{
	X,
	Y,
	Z
};

UENUM(BlueprintType, DisplayName = "Voxel 32 bit Mask", meta = (Bitflags))
enum class EVoxel32BitMask : uint8
{
	Channel0,
	Channel1,
	Channel2,
	Channel3,
	Channel4,
	Channel5,
	Channel6,
	Channel7,
	Channel8,
	Channel9,
	Channel10,
	Channel11,
	Channel12,
	Channel13,
	Channel14,
	Channel15,
	Channel16,
	Channel17,
	Channel18,
	Channel19,
	Channel20,
	Channel21,
	Channel22,
	Channel23,
	Channel24,
	Channel25,
	Channel26,
	Channel27,
	Channel28,
	Channel29,
	Channel30,
	Channel31
};

UENUM()
enum class EVoxelDataItemCombineMode
{
	Min,
	Max,
	Sum
};

UENUM(BlueprintType)
enum class EVoxelTaskType : uint8
{
	// Meshing of chunks that don't have collisions and are not visible
	ChunksMeshing,
	// Meshing of not visible chunks that have collisions
	CollisionsChunksMeshing,
	// Meshing of visible chunks that don't have collisions
	VisibleChunksMeshing,
	// Meshing of visible chunks that have collisions
	VisibleCollisionsChunksMeshing,
	// PhysX collision cooking, once the meshing task is done
	CollisionCooking,
	// Height spawners
	FoliageBuild,
	// Building of the instanced mesh components culling tree, used for spawners
	// The meshes are not updated until the build is done
	HISMBuild,
	// Async edit functions such as AddSphereAsync
	AsyncEditFunctions,
	// Mesh merge tasks are used after meshing to create the render buffers
	// Note: they are also used if bMergeChunks = false!
	MeshMerge,
	// The render octree is used to determine the LODs to display
	// Should be done as fast as possible to start meshing tasks 
	RenderOctree,
	
	Max UMETA(Hidden)
};

UENUM()
enum class EVoxelFoliageWorldType : uint8
{
	Flat,
	Sphere
};