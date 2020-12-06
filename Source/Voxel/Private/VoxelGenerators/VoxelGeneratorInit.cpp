// Copyright 2020 Phyronnaz

#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelGenerators/VoxelGeneratorCache.h"

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
	TVoxelSharedPtr<FVoxelGeneratorCache> PinnedGeneratorCache = GeneratorCache.Pin();
	if (!ensure(PinnedGeneratorCache))
	{
		PinnedGeneratorCache = FVoxelGeneratorCache::Create(*this);
	}
	return PinnedGeneratorCache.ToSharedRef();
}

void FVoxelGeneratorInit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(MaterialCollection);
}