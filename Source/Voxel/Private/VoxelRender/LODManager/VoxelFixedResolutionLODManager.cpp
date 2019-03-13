// Copyright 2019 Phyronnaz

#include "VoxelRender/LODManager/VoxelFixedResolutionLODManager.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelUtilities.h"

FVoxelFixedResolutionLODManager::FVoxelFixedResolutionLODManager(const FVoxelLODSettings& LODSettings, int ChunkLOD)
	: IVoxelLODManager(LODSettings)
{
	TArray<FVoxelChunkToAdd> ChunksToAdd;
	
	const int ChunkSize = FVoxelUtilities::GetSizeFromDepth<CHUNK_SIZE>(ChunkLOD);
	const FIntBox& WorldBounds = Settings.WorldBounds;

	FIntVector Min = FVoxelIntVector::FloorToInt(FVector(WorldBounds.Min) / ChunkSize) * ChunkSize;
	FIntVector Max = FVoxelIntVector::CeilToInt(FVector(WorldBounds.Max) / ChunkSize) * ChunkSize;
	
	uint64 Id = 0;
	for (int X = Min.X; X < Max.X; X += ChunkSize)
	{
		for (int Y = Min.Y; Y < Max.Y; Y += ChunkSize)
		{
			for (int Z = Min.Z; Z < Max.Z; Z += ChunkSize)
			{
				const FIntVector Position = FIntVector(X, Y, Z);
				const FIntBox ChunkBounds = FVoxelUtilities::GetBoundsFromPositionAndLOD<CHUNK_SIZE>(Position, ChunkLOD);
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

	Settings.Renderer->UpdateLODs(ChunksToAdd, {}, {}, {});
}