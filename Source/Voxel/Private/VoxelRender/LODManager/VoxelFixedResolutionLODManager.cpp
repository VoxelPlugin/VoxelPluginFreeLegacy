// Copyright 2020 Phyronnaz

#include "VoxelRender/LODManager/VoxelFixedResolutionLODManager.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelChunkToUpdate.h"
#include "VoxelUtilities/VoxelMathUtilities.h"

TVoxelSharedRef<FVoxelFixedResolutionLODManager> FVoxelFixedResolutionLODManager::Create(
	const FVoxelLODSettings& LODSettings)
{	
	return MakeShareable(new FVoxelFixedResolutionLODManager(LODSettings));
}

bool FVoxelFixedResolutionLODManager::Initialize(int32 ChunkLOD, int32 MaxChunks)
{
	TArray<FVoxelChunkUpdate> ChunkUpdates;
	
	const int32 ChunkSize = FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(ChunkLOD);
	const FVoxelIntBox& WorldBounds = Settings.WorldBounds;

	const FIntVector Min = FVoxelUtilities::FloorToInt(FVector(WorldBounds.Min) / ChunkSize) * ChunkSize;
	const FIntVector Max = FVoxelUtilities::CeilToInt(FVector(WorldBounds.Max) / ChunkSize) * ChunkSize;

	const FIntVector NumChunksPerAxis = (Max - Min) / ChunkSize;
	const int64 TotalNumChunks = int64(NumChunksPerAxis.X) * int64(NumChunksPerAxis.Y) * int64(NumChunksPerAxis.Z);

	if (TotalNumChunks > MaxChunks)
	{
		return false;
	}
	
	uint64 Id = 0;
	for (int32 X = Min.X; X < Max.X; X += ChunkSize)
	{
		for (int32 Y = Min.Y; Y < Max.Y; Y += ChunkSize)
		{
			for (int32 Z = Min.Z; Z < Max.Z; Z += ChunkSize)
			{
				const FIntVector Position = FIntVector(X, Y, Z);
				const FVoxelIntBox ChunkBounds = FVoxelUtilities::GetBoundsFromPositionAndDepth<RENDER_CHUNK_SIZE>(Position, ChunkLOD);
				if (WorldBounds.Intersect(ChunkBounds))
				{
					ChunkUpdates.Emplace(
						FVoxelChunkUpdate
						{
							Id++,
							ChunkLOD,
							ChunkBounds,
							{},
							FVoxelChunkSettings::VisibleWithCollisions(),
							{}
						});
				}
			}
		}
	}

	Settings.Renderer->UpdateLODs(1, ChunkUpdates);

	return true;
}
