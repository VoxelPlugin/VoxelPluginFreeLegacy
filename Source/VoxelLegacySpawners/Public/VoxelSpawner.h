// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelSpawnerConfig.h"
#include "VoxelSpawner.generated.h"

UCLASS(Abstract)
class VOXELLEGACYSPAWNERS_API UVoxelSpawner : public UVoxelLegacySpawnerBase
{
	GENERATED_BODY()

public:
	// Average distance between the instances, in voxels
	// Num Instances = Area in voxels / Square(DistanceBetweenInstancesInVoxel)
	// Not a density because the values would be too small to store in a float
	UPROPERTY(EditAnywhere, Category = "General Settings", meta = (ClampMin = 0))
	float DistanceBetweenInstancesInVoxel = 10;

	// Use this if you create the spawner at runtime
	UPROPERTY(Transient)
	uint32 SeedOverride = 0;
};