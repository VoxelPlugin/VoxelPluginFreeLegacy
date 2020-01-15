// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "IVoxelPool.generated.h"

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
	RenderOctree
};

class FVoxelQueuedThreadPool;
class FQueuedThreadPool;
class IVoxelQueuedWork;
class UWorld;

class VOXEL_API IVoxelPool
{
public:
	virtual ~IVoxelPool() {}

	//~ Begin IVoxelPool Interface
	virtual void QueueTask(EVoxelTaskType Type, IVoxelQueuedWork* Task) = 0;
	virtual void QueueTasks(EVoxelTaskType Type, const TArray<IVoxelQueuedWork*>& Tasks) = 0;

	virtual int32 GetNumTasks() const = 0;
	//~ End IVoxelPool Interface

public:
	static TVoxelSharedPtr<IVoxelPool> GetGlobalPool(UWorld* World);
	static const FString& GetGlobalPoolCreator(UWorld* World);
	static void DestroyGlobalVoxelPool(UWorld* World);
	static bool IsGlobalVoxelPoolCreated(UWorld* World);

	static void SetGlobalVoxelPool(UWorld* World, const TVoxelSharedPtr<IVoxelPool>& Pool, const FString& Creator);

private:
	struct FPool
	{
		TVoxelSharedPtr<IVoxelPool> Pool;
		FString Creator;
	};
	static TMap<TWeakObjectPtr<UWorld>, FPool> GlobalMap;
};