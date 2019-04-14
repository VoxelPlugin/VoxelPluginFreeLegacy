// Copyright 2019 Phyronnaz

#include "VoxelRender/LODManager/VoxelFixedResolutionLODManager.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelMathUtilities.h"

TSharedRef<FVoxelFixedResolutionLODManager> FVoxelFixedResolutionLODManager::Create(const FVoxelLODSettings& LODSettings, int32 ChunkLOD)
{
	TArray<FVoxelChunkToAdd> ChunksToAdd;
	
	const int32 ChunkSize = FVoxelUtilities::GetSizeFromDepth<CHUNK_SIZE>(ChunkLOD);
	const FIntBox& WorldBounds = LODSettings.WorldBounds;

	FIntVector Min = FVoxelUtilities::FloorToInt(FVector(WorldBounds.Min) / ChunkSize) * ChunkSize;
	FIntVector Max = FVoxelUtilities::CeilToInt(FVector(WorldBounds.Max) / ChunkSize) * ChunkSize;
	
	uint64 Id = 0;
	for (int32 X = Min.X; X < Max.X; X += ChunkSize)
	{
		for (int32 Y = Min.Y; Y < Max.Y; Y += ChunkSize)
		{
			for (int32 Z = Min.Z; Z < Max.Z; Z += ChunkSize)
			{
				const FIntVector Position = FIntVector(X, Y, Z);
				const FIntBox ChunkBounds = FVoxelUtilities::GetBoundsFromPositionAndDepth<CHUNK_SIZE>(Position, ChunkLOD);
				if (WorldBounds.Intersect(ChunkBounds))
				{
					ChunksToAdd.Emplace(FVoxelChunkToAdd{
						Id++,
						ChunkBounds,
						ChunkLOD,
						FVoxelRenderChunkSettings::Visible(),
						{} });
				}
			}
		}
	}

	LODSettings.Renderer->UpdateLODs(ChunksToAdd, {}, {}, {});
	
	return MakeShareable(new FVoxelFixedResolutionLODManager(LODSettings));
}