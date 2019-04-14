// Copyright 2019 Phyronnaz

#include "VoxelBenchmark.h"
#include "VoxelData/VoxelData.h"
#include "VoxelWorldGenerators/VoxelFlatWorldGenerator.h"
#include "VoxelRender/Polygonizers/VoxelMCPolygonizer.h"
#include "VoxelGlobals.h"

FAutoConsoleCommandWithWorldAndArgs CmdVoxelBasicBench(
	TEXT("voxel.benchmarks.BasicBench"),
	TEXT(""),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		int NumIter = 1;
		if (Args.Num() > 0 )
		{
			LexFromString(NumIter, *Args[0]);
		}
		bool bReuse = false;
		if (Args.Num() > 1)
		{
			LexFromString(bReuse, *Args[1]);
		}

		FVoxelBenchmark::BasicBench(NumIter, bReuse);
	})
);

void FVoxelBenchmark::BasicBench(int NumIter, bool bReuse)
{
	UE_LOG(LogVoxel, Warning, TEXT("Voxel basic benchmark: %d iters, reuse: %s"), NumIter, bReuse ? TEXT("yes") : TEXT("no"));

	FVoxelDataSettings DataSettings;
	DataSettings.Depth = 10;
	DataSettings.WorldGenerator = MakeShared<FVoxelFlatWorldGeneratorInstance, ESPMode::ThreadSafe>();
	DataSettings.ComputeBoundsFromDepth();
	auto Data = FVoxelData::Create(DataSettings);

	FVoxelChunk Chunk;
	FVoxelStatsElement Stats;

	FVoxelPolygonizerSettings PolygonizerSettings;
	PolygonizerSettings.LOD = 0;
	PolygonizerSettings.Data = &Data.Get();
	PolygonizerSettings.DebugManager = nullptr;
	PolygonizerSettings.NormalConfig = EVoxelNormalConfig::GradientNormal;
	PolygonizerSettings.MaterialConfig = EVoxelMaterialConfig::RGB;
	PolygonizerSettings.UVConfig = EVoxelUVConfig::GlobalUVs;
	PolygonizerSettings.UVScale = 1;
	PolygonizerSettings.MeshParameters = FVoxelMeshProcessingParameters(false, false);

	PolygonizerSettings.Chunk = &Chunk;
	PolygonizerSettings.Stats = &Stats;

	for (int Index = 0; Index < NumIter; Index++)
	{
		int M = bReuse ? 0 : Index;
		PolygonizerSettings.ChunkPosition = FIntVector(M * CHUNK_SIZE, M * CHUNK_SIZE, -CHUNK_SIZE);
		auto Polygonizer = MakeUnique<FVoxelMCPolygonizer>(PolygonizerSettings);
		Polygonizer->Create();
	}

	UE_LOG(LogVoxel, Warning, TEXT("Done"));
}