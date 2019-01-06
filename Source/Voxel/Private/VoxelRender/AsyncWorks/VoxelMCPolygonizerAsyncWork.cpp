// Copyright 2019 Phyronnaz

#include "VoxelMCPolygonizerAsyncWork.h"
#include "VoxelRender/Polygonizers/VoxelMCPolygonizer.h"

void FVoxelMCPolygonizerAsyncWork::DoWork()
{
	Stats.SetType(EVoxelStatsType::NormalMarchingCubes);

	Stats.StartStat("Polygonizer Creation");
	TSharedPtr<FVoxelMCPolygonizer> Builder = MakeShareable(new FVoxelMCPolygonizer(LOD, &Data.Get(), ChunkPosition, NormalConfig, MaterialConfig, UVConfig, MeshParameters));

	if (!Builder->CreateChunk(*Chunk, Stats))
	{
		ShowError();
	}

	Stats.SetValue("Canceled (polygonization was done for nothing)", IsCanceled() ? "1" : "0");

	
	Stats.StartStat("Waiting to be picked up by render chunk");
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelMCTransitionsPolygonizerAsyncWork::DoWork()
{
	Stats.SetType(EVoxelStatsType::TransitionsMarchingCubes);

	Stats.StartStat("Polygonizer Creation");
	TSharedPtr<FVoxelMCTransitionsPolygonizer> Builder = MakeShareable(new FVoxelMCTransitionsPolygonizer(LOD, &Data.Get(), ChunkPosition, TransitionsMask, NormalConfig, MaterialConfig, UVConfig, MeshParameters));
	
	if (!Builder->CreateTransitions(*Chunk, Stats))
	{
		ShowError();
	}
	
	Stats.StartStat("Waiting to be picked up by render chunk");
}