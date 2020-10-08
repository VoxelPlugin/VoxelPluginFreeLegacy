// Copyright 2020 Phyronnaz

#include "VoxelSpawners/VoxelSpawnerConfig.h"
#include "VoxelSpawners/VoxelSpawner.h"
#include "VoxelSpawners/VoxelSpawnerOutputsConfig.h"
#include "VoxelUtilities/VoxelMathUtilities.h"

#if WITH_EDITOR
bool UVoxelSpawnerConfig::NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (Object == GeneratorOutputs)
	{
		return true;
	}

	for (auto& Spawner : Spawners)
	{
		if (Spawner.Spawner == Object)
		{
			return true;
		}
		if (Spawner.Spawner && Spawner.Spawner->NeedsToRebuild(Object, PropertyChangedEvent))
		{
			return true;
		}
	}
	return false;
}

void UVoxelSpawnerConfig::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	SetReadOnlyPropertiesFromEditorOnly();
	SetEditorOnlyPropertiesFromReadOnly();
	FixGuids();
	FixSpawnerDensityTypes();
}
#endif

void UVoxelSpawnerConfig::PostLoad()
{
	Super::PostLoad();

	{
		const auto UpgradeDensityGraphOutputNameToDensityStruct = [&](auto& Groups)
		{
			for (auto& Group : Groups)
			{
				for (auto& Spawner : Group.Spawners)
				{
					if (!Spawner.DensityGraphOutputName_DEPRECATED.IsNone())
					{
						if (FName(Spawner.DensityGraphOutputName_DEPRECATED) == STATIC_FNAME("Constant 0"))
						{
							Spawner.Density.Type = EVoxelSpawnerDensityType::Constant;
							Spawner.Density.Constant = 0.f;
						}
						else if (FName(Spawner.DensityGraphOutputName_DEPRECATED) == STATIC_FNAME("Constant 1"))
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
	}

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

	SetEditorOnlyPropertiesFromReadOnly();
	FixGuids();
	FixSpawnerDensityTypes();
}

void UVoxelSpawnerConfig::SetReadOnlyPropertiesFromEditorOnly()
{
	for (auto& Spawner : Spawners)
	{
		Spawner.LOD = FVoxelUtilities::GetDepthFromSize<RENDER_CHUNK_SIZE>(Spawner.ChunkSize_EditorOnly);
		Spawner.GenerationDistanceInChunks = Spawner.GenerationDistanceInVoxels_EditorOnly / (RENDER_CHUNK_SIZE << Spawner.LOD);
		Spawner.GenerationDistanceInChunks = FMath::Max(1, Spawner.GenerationDistanceInChunks);
	}
}

void UVoxelSpawnerConfig::SetEditorOnlyPropertiesFromReadOnly()
{
	for (auto& Spawner : Spawners)
	{
		Spawner.ChunkSize_EditorOnly = RENDER_CHUNK_SIZE << Spawner.LOD;
		Spawner.GenerationDistanceInVoxels_EditorOnly = Spawner.GenerationDistanceInChunks * Spawner.ChunkSize_EditorOnly;
	}
}
void UVoxelSpawnerConfig::FixGuids()
{
	TSet<FGuid> Guids;

	for (auto& Spawner : Spawners)
	{
		if (!Spawner.Guid.IsValid())
		{
			Spawner.Guid = FGuid::NewGuid();
		}

		while (true)
		{
			bool bAlreadyInSet;
			Guids.Add(Spawner.Guid, &bAlreadyInSet);
			if (!bAlreadyInSet) break;
			Spawner.Guid = FGuid::NewGuid();
		}
	}
}

void UVoxelSpawnerConfig::FixSpawnerDensityTypes()
{
	for (auto& Spawner : Spawners)
	{
		if (Spawner.SpawnerType == EVoxelSpawnerType::Height && 
			Spawner.Density.Type != EVoxelSpawnerDensityType::Constant && 
			Spawner.Density.Type != EVoxelSpawnerDensityType::GeneratorOutput)
		{
			Spawner.Density.Type = EVoxelSpawnerDensityType::Constant;
		}
	}
}