// Copyright 2021 Phyronnaz

#include "VoxelFoliageBlueprintLibrary.h"
#include "VoxelFoliageInterface.h"
#include "VoxelHierarchicalInstancedStaticMeshComponent.h"
#include "VoxelTools/VoxelToolHelpers.h"

void UVoxelFoliageBlueprintLibrary::SpawnVoxelSpawnerActorsInArea(
	TArray<AVoxelFoliageActor*>& OutActors, 
	AVoxelWorld* World,
	FVoxelIntBox Bounds, 
	EVoxelSpawnerActorSpawnType SpawnType)
{
	FVoxelMessages::Info(FUNCTION_ERROR("Voxel Spawners require Voxel Plugin Pro"));
}

AVoxelFoliageActor* UVoxelFoliageBlueprintLibrary::SpawnVoxelSpawnerActorByInstanceIndex(
	AVoxelWorld* World, 
	UVoxelHierarchicalInstancedStaticMeshComponent* Component, 
	int32 InstanceIndex)
{
	FVoxelMessages::Info(FUNCTION_ERROR("Voxel Spawners require Voxel Plugin Pro"));
	return nullptr;
}

void UVoxelFoliageBlueprintLibrary::AddInstances(
	AVoxelWorld* World,
	UStaticMesh* Mesh,
	const TArray<FTransform>& Transforms,
	FVoxelInstancedMeshKey MeshKey,
	FVector FloatingDetectionOffset)
{
	FVoxelMessages::Info(FUNCTION_ERROR("Voxel Spawners require Voxel Plugin Pro"));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFoliageSave UVoxelFoliageBlueprintLibrary::GetSpawnersSave(AVoxelWorld* World)
{
	FVoxelMessages::Info(FUNCTION_ERROR("Voxel Spawners require Voxel Plugin Pro"));
	return {};
}

void UVoxelFoliageBlueprintLibrary::LoadFromSpawnersSave(AVoxelWorld* World, const FVoxelFoliageSave& Save)
{
	FVoxelMessages::Info(FUNCTION_ERROR("Voxel Spawners require Voxel Plugin Pro"));
}
