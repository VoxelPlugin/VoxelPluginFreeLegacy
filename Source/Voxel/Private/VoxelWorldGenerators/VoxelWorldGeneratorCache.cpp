// Copyright 2020 Phyronnaz

#include "VoxelWorldGenerators/VoxelWorldGeneratorCache.h"
#include "VoxelWorldGenerators/VoxelWorldGenerator.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorInstance.h"

FVoxelWorldGeneratorCache::FVoxelWorldGeneratorCache(const FVoxelWorldGeneratorInit& WorldGeneratorInit)
	: WorldGeneratorInit(WorldGeneratorInit)
{
}

TVoxelSharedRef<FVoxelWorldGeneratorInstance> FVoxelWorldGeneratorCache::CreateWorldGeneratorInstance(UVoxelWorldGenerator& Generator)
{
	auto& Instance = WorldGeneratorCache.FindOrAdd(&Generator);
	if (!Instance.IsValid())
	{
		Instance = Generator.GetInstance();
		Instance->Init(WorldGeneratorInit);
	}
	return Instance.ToSharedRef();
}

void FVoxelWorldGeneratorCache::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(WorldGeneratorInit.MaterialCollection);	
	Collector.AddReferencedObject(WorldGeneratorInit.World);	
}