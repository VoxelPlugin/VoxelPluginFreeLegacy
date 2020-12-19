// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSubsystem.h"
#include "VoxelSpawnerManagerBase.generated.h"

class UVoxelFoliageBiome;

UCLASS(Abstract)
class VOXEL_API UVoxelSpawnerManagerBaseSubsystemProxy : public UVoxelStaticSubsystemProxy
{
	GENERATED_BODY()
	GENERATED_ABSTRACT_VOXEL_SUBSYSTEM_PROXY_BODY(IVoxelSpawnerManagerBase);
};

class IVoxelSpawnerManagerBase : public IVoxelSubsystem
{
public:
	GENERATED_VOXEL_SUBSYSTEM_BODY(UVoxelSpawnerManagerBaseSubsystemProxy);

	// Returns the biome index to output in the biome generator output
	virtual int32 RegisterBiome(UVoxelFoliageBiome* Biome) { return -1; }
};