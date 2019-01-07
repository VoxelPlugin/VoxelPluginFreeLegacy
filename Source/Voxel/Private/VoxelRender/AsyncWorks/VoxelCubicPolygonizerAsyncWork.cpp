// Copyright 2019 Phyronnaz

#include "VoxelCubicPolygonizerAsyncWork.h"
#include "VoxelRender/Polygonizers/VoxelCubicPolygonizer.h"

void FVoxelCubicPolygonizerAsyncWork::DoWork()
{
	Stats.SetType(EVoxelStatsType::NormalCubic);

	Stats.StartStat("Polygonizer Creation");
	TSharedPtr<FVoxelCubicPolygonizer> Builder = MakeShareable(new FVoxelCubicPolygonizer(
		LOD, 
		&Data.Get(), 
		ChunkPosition, 
		MaterialConfig, 
		UVConfig,
		bCacheLOD0Chunks,
		MeshParameters));

	if (!Builder->CreateSection(*Chunk, Stats))
	{
		ShowError();
	}

	Stats.SetValue("Canceled (polygonization was done for nothing)", IsCanceled() ? "1" : "0");

	
	Stats.StartStat("Waiting to be picked up by render chunk");
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelCubicTransitionsPolygonizerAsyncWork::DoWork()
{
	Stats.SetType(EVoxelStatsType::TransitionsCubic);

	Stats.StartStat("Polygonizer Creation");
	TSharedPtr<FVoxelCubicTransitionsPolygonizer> Builder = MakeShareable(new FVoxelCubicTransitionsPolygonizer(LOD, &Data.Get(), ChunkPosition, TransitionsMask, MaterialConfig, UVConfig, MeshParameters));
	
	if (!Builder->CreateTransitions(*Chunk, Stats))
	{
		ShowError();
	}
	
	Stats.StartStat("Waiting to be picked up by render chunk");
}