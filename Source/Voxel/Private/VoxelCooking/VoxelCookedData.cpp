// Copyright 2020 Phyronnaz

#include "VoxelCooking/VoxelCookedData.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelCookedDataMemory);

FVoxelCookedDataImpl::~FVoxelCookedDataImpl()
{
}

bool FVoxelCookedDataImpl::Serialize(FArchive& Ar)
{
	if ((Ar.IsLoading() || Ar.IsSaving()) && !Ar.IsTransacting())
	{
		if (Ar.IsSaving())
		{
			Version = FVoxelCookedDataVersion::LatestVersion;
		}

		Ar << Version;
		Ar << Guid;
		Ar << Chunks;		
		
		UpdateAllocatedSize();
	}

	return true;
}

void FVoxelCookedDataImpl::UpdateAllocatedSize() const
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCookedDataMemory, AllocatedSize);
	AllocatedSize = Chunks.GetAllocatedSize();
	for (auto& Chunk : Chunks)
	{
		AllocatedSize += Chunk.Data.GetAllocatedSize();
	}
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCookedDataMemory, AllocatedSize);
}

void FVoxelCookedDataImpl::RemoveEmptyChunks()
{
	Chunks.RemoveAllSwap([](auto& Chunk) { return Chunk.Data.Num() == 0; });
}
