// Copyright 2020 Phyronnaz

#include "VoxelSpawners/VoxelSpawnerConfig.h"
#include "VoxelSpawners/VoxelSpawner.h"
#include "VoxelSpawners/VoxelSpawnerOutputsConfig.h"
#include "VoxelUtilities/VoxelMathUtilities.h"

#if WITH_EDITOR
bool FVoxelSpawnerDensity::NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const
{
	return Type == EVoxelSpawnerDensityType::GeneratorOutput && !bUseMainGenerator && CustomGenerator.GetObject() == Object;
}

bool UVoxelSpawnerCollection::NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const
{
	if (Object == MainGeneratorForDropdowns.GetObject())
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
		if (Spawner.Density.NeedsToRebuild(Object, PropertyChangedEvent))
		{
			return true;
		}
		if (Spawner.DensityMultiplier.NeedsToRebuild(Object, PropertyChangedEvent))
		{
			return true;
		}
	}
	return false;
}

void UVoxelSpawnerCollection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	SetReadOnlyPropertiesFromEditorOnly();
	SetEditorOnlyPropertiesFromReadOnly();
	FixGuids();
}
#endif

void UVoxelSpawnerCollection::PostLoad()
{
	Super::PostLoad();

	SetEditorOnlyPropertiesFromReadOnly();
	FixGuids();
}

void UVoxelSpawnerCollection::SetReadOnlyPropertiesFromEditorOnly()
{
	for (auto& Spawner : Spawners)
	{
		Spawner.LOD = FVoxelUtilities::GetDepthFromSize<RENDER_CHUNK_SIZE>(Spawner.ChunkSize_EditorOnly);
		Spawner.GenerationDistanceInChunks = Spawner.GenerationDistanceInVoxels_EditorOnly / (RENDER_CHUNK_SIZE << Spawner.LOD);
		Spawner.GenerationDistanceInChunks = FMath::Max(1, Spawner.GenerationDistanceInChunks);
	}
}

void UVoxelSpawnerCollection::SetEditorOnlyPropertiesFromReadOnly()
{
	for (auto& Spawner : Spawners)
	{
		Spawner.ChunkSize_EditorOnly = RENDER_CHUNK_SIZE << Spawner.LOD;
		Spawner.GenerationDistanceInVoxels_EditorOnly = Spawner.GenerationDistanceInChunks * Spawner.ChunkSize_EditorOnly;
	}
}

void UVoxelSpawnerCollection::FixGuids()
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

#if WITH_EDITOR
bool UVoxelSpawnerConfig::NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const
{
	for (auto& Collection : Collections)
	{
		if (Object == Collection)
		{
			return true;
		}
		if (Collection && Collection->NeedsToRebuild(Object, PropertyChangedEvent))
		{
			return true;
		}
	}

	return Super::NeedsToRebuild(Object, PropertyChangedEvent);
}
#endif