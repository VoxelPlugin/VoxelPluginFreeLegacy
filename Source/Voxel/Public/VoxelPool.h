// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelSubsystem.h"
#include "VoxelPool.generated.h"

class IVoxelQueuedWork;

DECLARE_UNIQUE_VOXEL_ID(FVoxelPoolId);

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

UCLASS()
class VOXEL_API UVoxelPoolSubsystemProxy : public UVoxelStaticSubsystemProxy
{
	GENERATED_BODY()
	GENERATED_VOXEL_SUBSYSTEM_PROXY_BODY(FVoxelPool);
};

class VOXEL_API FVoxelPool : public IVoxelSubsystem
{
public:
	GENERATED_VOXEL_SUBSYSTEM_BODY(UVoxelPoolSubsystemProxy);

	const FVoxelPoolId PoolId = FVoxelPoolId::New();
	
	//~ Begin IVoxelSubsystem Interface
	virtual void Create() override;
	//~ End IVoxelSubsystem Interface
	
	void QueueTask(IVoxelQueuedWork* Task) const;
	void QueueTasks(EVoxelTaskType Type, const TArray<IVoxelQueuedWork*>& Tasks) const;
	
private:
	TVoxelStaticArray<uint32, 256> PriorityCategories;
	TVoxelStaticArray<uint32, 256> PriorityOffsets;

public:
	static void FixPriorityCategories(TMap<EVoxelTaskType, int32>& PriorityCategories);
	static void FixPriorityOffsets(TMap<EVoxelTaskType, int32>& PriorityOffsets);
};