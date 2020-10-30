// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelData/IVoxelData.h"

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Custom Channels Memory"), STAT_VoxelCustomChannelsMemory, STATGROUP_VoxelMemory, VOXEL_API);
DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Custom Channels Maps Memory"), STAT_VoxelCustomChannelsMapsMemory, STATGROUP_VoxelMemory, VOXEL_API);

class VOXEL_API FVoxelDataOctreeLeafCustomChannels
{
	TMap<FName, uint8* RESTRICT> Data;
	
	friend class FVoxelSaveBuilder;
	friend class FVoxelSaveLoader;

public:
	FVoxelDataOctreeLeafCustomChannels()
	{
		INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCustomChannelsMapsMemory, Data.GetAllocatedSize());
	}
	~FVoxelDataOctreeLeafCustomChannels()
	{
		if (!ensureVoxelSlow(Data.Num() == 0))
		{
			ClearData(IVoxelDataOctreeMemory());
		}
		
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCustomChannelsMapsMemory, Data.GetAllocatedSize());
	}

	UE_NONCOPYABLE(FVoxelDataOctreeLeafCustomChannels);

public:
	uint8* RESTRICT CreateData(const IVoxelDataOctreeMemory& Memory, FName Name, bool bMemzero = true);
	void ClearData(const IVoxelDataOctreeMemory& Memory);

	FORCEINLINE uint8* RESTRICT GetData(FName Name)
	{
		return Data.FindRef(Name);
	}
	FORCEINLINE const uint8* RESTRICT GetData(FName Name) const
	{
		return Data.FindRef(Name);
	}
	
private:
	static constexpr int32 MemorySize = VOXELS_PER_DATA_CHUNK * sizeof(uint8);

	static uint8* Allocate(const IVoxelDataOctreeMemory& Memory);
	static void Deallocate(const IVoxelDataOctreeMemory& Memory, uint8* Ptr);
};