// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSubsystem.h"

// Fired once per chunk
DECLARE_MULTICAST_DELEGATE_OneParam(FVoxelOnChunkUpdateFinished, FVoxelIntBox);
DECLARE_MULTICAST_DELEGATE_OneParam(FVoxelOnChunkUpdate, FVoxelIntBox);

class VOXEL_API IVoxelLODManager : public IVoxelSubsystem
{
public:
	GENERATED_VOXEL_SUBSYSTEM_BODY(UVoxelLODSubsystemProxy);
	
	FVoxelOnChunkUpdate OnChunkUpdate;

	//~ Begin IVoxelLODManager Interface
	// Both specializations are used, and we don't want to allocate single element arrays or to do lots of virtual calls
	// Returns the number of chunks to update = number of times FinishDelegate is going to be fired
	virtual int32 UpdateBounds(const FVoxelIntBox& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate = FVoxelOnChunkUpdateFinished()) = 0;
	virtual int32 UpdateBounds(const TArray<FVoxelIntBox>& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate = FVoxelOnChunkUpdateFinished()) = 0;

	virtual void ForceLODsUpdate() = 0;
	virtual bool AreCollisionsEnabled(const FIntVector& Position, uint8& OutLOD) const = 0;
	//~ End IVoxelLODManager Interface
	
public:
	int32 UpdateBounds(const FVoxelIntBox& Bounds, const FVoxelOnChunkUpdateFinished::FDelegate& FinishDelegate)
	{
		FVoxelOnChunkUpdateFinished MulticastDelegate;
		MulticastDelegate.Add(FinishDelegate);
		return UpdateBounds(Bounds, MulticastDelegate);
	}
	int32 UpdateBounds(const TArray<FVoxelIntBox>& Bounds, const FVoxelOnChunkUpdateFinished::FDelegate& FinishDelegate)
	{
		FVoxelOnChunkUpdateFinished MulticastDelegate;
		MulticastDelegate.Add(FinishDelegate);
		return UpdateBounds(Bounds, MulticastDelegate);
	}

	template<typename T>
	int32 UpdateBounds_OnAllFinished(const T& Bounds, const FSimpleDelegate& AllFinishedDelegate, const FVoxelOnChunkUpdateFinished::FDelegate& FinishDelegate = {})
	{
		TVoxelSharedRef<int32> Count = MakeVoxelShared<int32>(0);
		*Count = UpdateBounds(
			Bounds,
			FVoxelOnChunkUpdateFinished::FDelegate::CreateLambda([=](FVoxelIntBox ChunkBounds)
				{
					(*Count)--;
					ensure(*Count >= 0);
					FinishDelegate.ExecuteIfBound(ChunkBounds);
					if (*Count == 0)
					{
						AllFinishedDelegate.ExecuteIfBound();
					}
				}));
		if (*Count == 0)
		{
			// No chunk to update
			AllFinishedDelegate.ExecuteIfBound();
		}
		return *Count;
	}
};