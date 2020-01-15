// Copyright 2020 Phyronnaz

#include "VoxelRender/LODManager/VoxelFixedResolutionLODManager.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelChunkToUpdate.h"
#include "VoxelMathUtilities.h"

TVoxelSharedRef<FVoxelFixedResolutionLODManager> FVoxelFixedResolutionLODManager::Create(
	const FVoxelLODSettings& LODSettings,
	int32 ChunkLOD)
{
	TArray<FVoxelChunkUpdate> ChunkUpdates;
	
	const int32 ChunkSize = FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(ChunkLOD);
	const FIntBox& WorldBounds = LODSettings.WorldBounds;

	const FIntVector Min = FVoxelUtilities::FloorToInt(FVector(WorldBounds.Min) / ChunkSize) * ChunkSize;
	const FIntVector Max = FVoxelUtilities::CeilToInt(FVector(WorldBounds.Max) / ChunkSize) * ChunkSize;
	
	uint64 Id = 0;
	for (int32 X = Min.X; X < Max.X; X += ChunkSize)
	{
		for (int32 Y = Min.Y; Y < Max.Y; Y += ChunkSize)
		{
			for (int32 Z = Min.Z; Z < Max.Z; Z += ChunkSize)
			{
				const FIntVector Position = FIntVector(X, Y, Z);
				const FIntBox ChunkBounds = FVoxelUtilities::GetBoundsFromPositionAndDepth<RENDER_CHUNK_SIZE>(Position, ChunkLOD);
				if (WorldBounds.Intersect(ChunkBounds))
				{
					ChunkUpdates.Emplace(
						FVoxelChunkUpdate
						{
							Id++,
							ChunkLOD,
							ChunkBounds,
							{},
							FVoxelChunkSettings::VisibleWithCollisions(false),
							{}
						});
				}
			}
		}
	}

	LODSettings.Renderer->UpdateLODs(1, ChunkUpdates);
	
	return MakeShareable(new FVoxelFixedResolutionLODManager(LODSettings));
}