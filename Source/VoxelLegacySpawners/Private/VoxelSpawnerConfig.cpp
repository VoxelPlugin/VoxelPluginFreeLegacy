// Copyright 2021 Phyronnaz

#include "VoxelSpawnerConfig.h"
#include "VoxelUtilities/VoxelMathUtilities.h"

void UVoxelSpawnerConfig::PostLoad()
{
	Super::PostLoad();

	const auto UpgradeDensityGraphOutputNameToDensityStruct = [&](auto& Groups)
	{
		for (auto& Group : Groups)
		{
			for (auto& Spawner : Group.Spawners)
			{
				if (!Spawner.DensityGraphOutputName_DEPRECATED.Name.IsNone())
				{
					if (Spawner.DensityGraphOutputName_DEPRECATED.Name == STATIC_FNAME("Constant 0"))
					{
						Spawner.Density.Type = EVoxelSpawnerDensityType::Constant;
						Spawner.Density.Constant = 0.f;
					}
					else if (Spawner.DensityGraphOutputName_DEPRECATED.Name == STATIC_FNAME("Constant 1"))
					{
						Spawner.Density.Type = EVoxelSpawnerDensityType::Constant;
						Spawner.Density.Constant = 1.f;
					}
					else
					{
						Spawner.Density.Type = EVoxelSpawnerDensityType::GeneratorOutput;
						Spawner.Density.GeneratorOutputName = Spawner.DensityGraphOutputName_DEPRECATED;
					}
				}
			}
		}
	};

	UpgradeDensityGraphOutputNameToDensityStruct(HeightSpawners_DEPRECATED);
	UpgradeDensityGraphOutputNameToDensityStruct(RaySpawners_DEPRECATED);

	for (const FVoxelSpawnerConfigRayGroup& Group : RaySpawners_DEPRECATED)
	{
		for (const FVoxelSpawnerConfigElement_Ray& Spawner : Group.Spawners)
		{
			FVoxelSpawnerConfigSpawner NewSpawner;
			
			NewSpawner.Spawner = Spawner.Spawner;
			NewSpawner.SpawnerType = EVoxelSpawnerType::Ray;
			NewSpawner.Density = Spawner.Density;
			NewSpawner.DensityMultiplier_RayOnly = Spawner.DensityMultiplier;
			NewSpawner.HeightGraphOutputName_HeightOnly = "";
			NewSpawner.LOD = Group.LOD;
			NewSpawner.GenerationDistanceInChunks = Group.GenerationDistanceInChunks;
			
			NewSpawner.bSave = Spawner.Advanced.bSave;
			NewSpawner.bDoNotDespawn = Spawner.Advanced.bDoNotDespawn;
			NewSpawner.Seed = Spawner.Advanced.DefaultSeed;
			NewSpawner.RandomGenerator = Spawner.Advanced.RandomGenerator;
			NewSpawner.Guid = Spawner.Advanced.Guid;
			NewSpawner.bComputeDensityFirst_HeightOnly = false;

			Spawners.Add(NewSpawner);
		}
	}
	RaySpawners_DEPRECATED.Reset();

	for (const FVoxelSpawnerConfigHeightGroup& Group : HeightSpawners_DEPRECATED)
	{
		for (const FVoxelSpawnerConfigElement_Height& Spawner : Group.Spawners)
		{
			FVoxelSpawnerConfigSpawner NewSpawner;
			
			NewSpawner.Spawner = Spawner.Spawner;
			NewSpawner.SpawnerType = EVoxelSpawnerType::Height;
			NewSpawner.Density = Spawner.Density;
			NewSpawner.DensityMultiplier_RayOnly = {};
			NewSpawner.HeightGraphOutputName_HeightOnly = Group.HeightGraphOutputName;
			NewSpawner.LOD = FVoxelUtilities::GetDepthFromSize<RENDER_CHUNK_SIZE>(Group.ChunkSize);
			NewSpawner.GenerationDistanceInChunks = Group.GenerationDistanceInChunks;
			
			NewSpawner.bSave = Spawner.Advanced.bSave;
			NewSpawner.bDoNotDespawn = Spawner.Advanced.bDoNotDespawn;
			NewSpawner.Seed = Spawner.Advanced.DefaultSeed;
			NewSpawner.RandomGenerator = Spawner.Advanced.RandomGenerator;
			NewSpawner.Guid = Spawner.Advanced.Guid;
			NewSpawner.bComputeDensityFirst_HeightOnly = Spawner.Advanced.bComputeDensityFirst;

			Spawners.Add(NewSpawner);
		}
	}
	HeightSpawners_DEPRECATED.Reset();
}