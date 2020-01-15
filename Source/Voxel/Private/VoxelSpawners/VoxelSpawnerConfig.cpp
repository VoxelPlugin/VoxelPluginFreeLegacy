// Copyright 2020 Phyronnaz

#include "VoxelSpawners/VoxelSpawnerConfig.h"
#include "VoxelGlobals.h"

#if WITH_EDITOR
void UVoxelSpawnerConfig::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_STATIC(FVoxelSpawnerConfigHeightGroup, ChunkSize))
	{
		for (auto& Element : HeightSpawners)
		{
			Element.ChunkSize = FMath::RoundToInt(Element.ChunkSize / 16.f) * 16;
		}
	}

	UpdateReadOnlyProperties();
}

void UVoxelSpawnerConfig::PostLoad()
{
	Super::PostLoad();

	UpdateReadOnlyProperties();
}

void UVoxelSpawnerConfig::UpdateReadOnlyProperties()
{
	for (auto& Element : HeightSpawners)
	{
		Element.GenerationDistanceInVoxels = Element.ChunkSize * Element.GenerationDistanceInChunks;
	}
	for (auto& Element : RaySpawners)
	{
		Element.ChunkSize = RENDER_CHUNK_SIZE << Element.LOD;
		Element.GenerationDistanceInVoxels = Element.ChunkSize * Element.GenerationDistanceInChunks;
	}
}
#endif