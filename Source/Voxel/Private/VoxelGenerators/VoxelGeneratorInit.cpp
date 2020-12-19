// Copyright 2020 Phyronnaz

#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelGenerators/VoxelGeneratorCache.h"
#include "VoxelFoliage/VoxelSpawnerManagerBase.h"

FVoxelGeneratorInit::FVoxelGeneratorInit(
	float VoxelSize, 
	uint32 WorldSize, 
	EVoxelRenderType RenderType, 
	EVoxelMaterialConfig MaterialConfig, 
	const UVoxelMaterialCollectionBase* MaterialCollection, 
	const TWeakObjectPtr<const AVoxelWorld>& World)
	: VoxelSize(VoxelSize)
	, WorldSize(WorldSize)
	, RenderType(RenderType)
	, MaterialConfig(MaterialConfig)
	, MaterialCollection(MaterialCollection)
	, World(World)
{
}

TVoxelSharedRef<FVoxelGeneratorCache> FVoxelGeneratorInit::GetGeneratorCache() const
{
	const TVoxelSharedPtr<FVoxelGeneratorCache> Pinned = GeneratorCache.Pin();
	if (ensure(Pinned))
	{
		return Pinned.ToSharedRef();
	}
	else
	{
		return FVoxelGeneratorCache::Create(*this);
	}
}

TVoxelSharedPtr<IVoxelSpawnerManagerBase> FVoxelGeneratorInit::GetSpawnerManager() const
{
	return SpawnerManager.Pin();
}

void FVoxelGeneratorInit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(MaterialCollection);
}