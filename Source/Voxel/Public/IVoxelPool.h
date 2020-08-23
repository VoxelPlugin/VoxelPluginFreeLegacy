// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
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

namespace EVoxelTaskType_DefaultPriorityCategories
{
	enum Type : int32
	{
		Min                            = 0,
		Max                            = 1000000,
		
		ChunksMeshing                  = 0,
		CollisionsChunksMeshing        = 1,
		VisibleChunksMeshing           = 10,
		VisibleCollisionsChunksMeshing = 100,
		CollisionCooking               = 100,
		FoliageBuild                   = 100,
		HISMBuild                      = 1000,
		AsyncEditFunctions             = 50,
		MeshMerge                      = 100000,
		RenderOctree                   = 1000000
	};
}

namespace EVoxelTaskType_DefaultPriorityOffsets
{
	enum Type : int32
	{
		ChunksMeshing                  = 0,
		CollisionsChunksMeshing        = 0,
		VisibleChunksMeshing           = 0,
		// By default, do collision cooking slightly before collision meshing, and foliage slightly after
		VisibleCollisionsChunksMeshing = 0,
		CollisionCooking               = +32,
		FoliageBuild                   = -32,
		HISMBuild                      = 0,
		AsyncEditFunctions             = 0,
		MeshMerge                      = 0,
		RenderOctree                   = 0
	};
}

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
	static TVoxelSharedPtr<IVoxelPool> GetWorldPool(UWorld* World);
	static TVoxelSharedPtr<IVoxelPool> GetGlobalPool();

	static TVoxelSharedPtr<IVoxelPool> GetPoolForWorld(UWorld* World);
	
public:
	static void SetWorldPool(UWorld* World, const TVoxelSharedRef<IVoxelPool>& Pool, const FString& Creator);
	static void SetGlobalPool(const TVoxelSharedRef<IVoxelPool>& Pool, const FString& Creator);
	
public:
	static void DestroyWorldPool(UWorld* World);
	static void DestroyGlobalPool();

	static void Shutdown();
	
private:
	static TMap<TWeakObjectPtr<UWorld>, TVoxelSharedPtr<IVoxelPool>> WorldsPools;
	static TVoxelSharedPtr<IVoxelPool> GlobalPool;
};