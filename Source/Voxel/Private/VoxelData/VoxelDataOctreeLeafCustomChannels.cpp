// Copyright 2021 Phyronnaz

#include "VoxelData/VoxelDataOctreeLeafCustomChannels.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelCustomChannelsMemory);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelCustomChannelsMapsMemory);

uint8* RESTRICT FVoxelDataOctreeLeafCustomChannels::CreateData(const IVoxelDataOctreeMemory& Memory, FName Name, bool bMemzero)
{
	check(!GetData(Name));

	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCustomChannelsMapsMemory, Data.GetAllocatedSize());

	auto* NewData = Allocate(Memory);

	Data.Add(Name, NewData);
	Data.Compact(); // Save as much memory as possible

	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCustomChannelsMapsMemory, Data.GetAllocatedSize());

	if (bMemzero)
	{
		FMemory::Memzero(NewData, MemorySize);
	}
	return NewData;
}

void FVoxelDataOctreeLeafCustomChannels::ClearData(const IVoxelDataOctreeMemory& Memory)
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCustomChannelsMapsMemory, Data.GetAllocatedSize());

	for (auto& It : Data)
	{
		Deallocate(Memory, It.Value);
	}
	Data.Empty();

	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCustomChannelsMapsMemory, Data.GetAllocatedSize());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint8* FVoxelDataOctreeLeafCustomChannels::Allocate(const IVoxelDataOctreeMemory& Memory)
{
	VOXEL_SLOW_FUNCTION_COUNTER();

	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCustomChannelsMemory, MemorySize);
	Memory.CustomChannelsMemory.Add(MemorySize);

	return static_cast<uint8*>(FMemory::Malloc(MemorySize));
}

void FVoxelDataOctreeLeafCustomChannels::Deallocate(const IVoxelDataOctreeMemory& Memory, uint8* Ptr)
{
	VOXEL_SLOW_FUNCTION_COUNTER();

	check(Ptr);

	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCustomChannelsMemory, MemorySize);
	Memory.CustomChannelsMemory.Subtract(MemorySize);
	ensure(Memory.CustomChannelsMemory.GetValue() >= 0);

	FMemory::Free(Ptr);
}