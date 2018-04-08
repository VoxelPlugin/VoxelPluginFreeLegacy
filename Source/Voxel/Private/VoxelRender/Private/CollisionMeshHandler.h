// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelProceduralMeshComponent.h"
#include "IntBox.h"

class AVoxelWorld;
class UVoxelInvokerComponent;
class FVoxelData;
class FQueuedThreadPool;

class FAsyncCollisionTask : public FNonAbandonableTask
{
public:
	// Output
	FVoxelProcMeshSection Section;

	const bool bEnableRender;
	const FIntVector ChunkPosition;
	FVoxelData* const Data;

	FAsyncCollisionTask(FVoxelData* Data, FIntVector ChunkPosition, bool bEnableRender);

	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncFoliageTask, STATGROUP_ThreadPoolAsyncTasks);
	};
};

class VOXEL_API FCollisionMeshHandler
{
public:
	TWeakObjectPtr<UVoxelInvokerComponent> const Invoker;
	AVoxelWorld* const World;
	FQueuedThreadPool* const Pool;

	FCollisionMeshHandler(TWeakObjectPtr<UVoxelInvokerComponent> Invoker, AVoxelWorld* World, AActor* ChunksOwner, FQueuedThreadPool* Pool);
	~FCollisionMeshHandler();

	void StartTasksTick();
	void EndTasksTick();

	FORCEINLINE bool IsValid();

	void Destroy();

	void UpdateInBox(const FIntBox& Box);

private:
	FIntVector CurrentCenter;
	UVoxelProceduralMeshComponent* Components[2][2][2];
	FAsyncTask<FAsyncCollisionTask>* Tasks[2][2][2];
	TSet<FIntVector> ChunksToUpdate;

	void Update(int X, int Y, int Z);
};