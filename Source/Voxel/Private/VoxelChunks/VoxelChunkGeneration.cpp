// Copyright 2021 Phyronnaz

#include "VoxelChunks/VoxelChunkGeneration.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"

DEFINE_VOXEL_SUBSYSTEM_PROXY(UVoxelChunksSubsystemProxy);

void FVoxelChunksSubsystem::Create()
{
	Super::Create();

	const TVoxelSharedRef<FVoxelChunkData> Chunk = Data->FindOrAddChunk(FIntPoint::ZeroValue);

	FVoxelChunkData::TData<FVoxelValue> Values;
	for (FVoxelValue& Value : Values)
	{
		Value = FVoxelValue::Full();
	}
	Chunk->SetData(Values);
}