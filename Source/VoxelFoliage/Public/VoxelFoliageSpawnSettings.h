// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelDistance.h"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelGenerators/VoxelGeneratorOutputPicker.h"
#include "VoxelFoliageSpawnSettings.generated.h"

UENUM()
enum class EVoxelFoliageSpawnType
{
	// Will line trace the voxel geometry to find spawning locations. Works with any kind of world/shapes
	Ray,
	// This uses a height output from the generator to spawn, allowing for large spawn distance.
	Height
};

UENUM()
enum class EVoxelFoliageRandomGenerator : uint8
{
	// Evenly distributed points
	Sobol,
	// More uneven points than Sobol. Unreal uses Halton to spawn grass in the default Landscape system
	Halton
};

USTRUCT(BlueprintType)
struct FVoxelFoliageSpawnSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	EVoxelFoliageSpawnType SpawnType = EVoxelFoliageSpawnType::Ray;
	
	// Average distance between the instances
	// Num Instances = Area in voxels / Square(DistanceBetweenInstancesInVoxel)
	// Not a density because the values would be too small to store in a float
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVoxelDistance DistanceBetweenInstances = FVoxelDistance::Voxels(10);
	
	// Chunk size
	// SpawnType = Height: make that as big as possible
	// SpawnType = Ray: bigger values will improve performance at the cost of accuracy, as a lower resolution mesh will be used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (DisplayName = "Chunk Size", ClampMin = 32))
	int32 ChunkSize = 32;

	// Generation distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Spawn", meta = (DisplayName = "Generation Distance"))
	FVoxelDistance GenerationDistance = FVoxelDistance::Voxels(32);
	
	// Controls the spawning pattern
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	EVoxelFoliageRandomGenerator RandomGenerator = EVoxelFoliageRandomGenerator::Halton;

	// Will spawn foliage on the entire map, with no distance limit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Spawn")
	bool bInfiniteGenerationDistance = false;

	// The name of the custom graph output used to determine the height
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (DisplayName = "Height Graph Output Name (height only)"))
	FVoxelGeneratorOutputPicker HeightGraphOutputName_HeightOnly = "Height";

	// If true, will not spawn height instances if they are now floating due to user edits or additional 3D noise in the generator
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Spawn", meta = (DisplayName = "Check If Floating (height only)"))
	bool bCheckIfFloating_HeightOnly = true;

	// If true, will not spawn height instances if they are now covered due to user edits or additional 3D noise in the generator
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Spawn", meta = (DisplayName = "Check If Covered (height only)"))
	bool bCheckIfCovered_HeightOnly = true;

public:
	int32 GetLOD() const
	{
		return FMath::Max(0, FVoxelUtilities::GetDepthFromSize(FOLIAGE_CHUNK_SIZE, ChunkSize));
	}
	int32 GetChunkSize() const
	{
		return FOLIAGE_CHUNK_SIZE << GetLOD();
	}
	int32 GetGenerationDistanceInChunks(float VoxelSize) const
	{
		return FMath::CeilToInt(GenerationDistance.GetInVoxels(VoxelSize) / GetChunkSize());
	}

	friend bool operator==(const FVoxelFoliageSpawnSettings& Lhs, const FVoxelFoliageSpawnSettings& Rhs)
	{
		return Lhs.SpawnType == Rhs.SpawnType
			&& Lhs.DistanceBetweenInstances == Rhs.DistanceBetweenInstances
			&& Lhs.ChunkSize == Rhs.ChunkSize
			&& Lhs.GenerationDistance == Rhs.GenerationDistance
			&& Lhs.RandomGenerator == Rhs.RandomGenerator
			&& Lhs.bInfiniteGenerationDistance == Rhs.bInfiniteGenerationDistance
			&& Lhs.HeightGraphOutputName_HeightOnly == Rhs.HeightGraphOutputName_HeightOnly
			&& Lhs.bCheckIfFloating_HeightOnly == Rhs.bCheckIfFloating_HeightOnly
			&& Lhs.bCheckIfCovered_HeightOnly == Rhs.bCheckIfCovered_HeightOnly;
	}
};