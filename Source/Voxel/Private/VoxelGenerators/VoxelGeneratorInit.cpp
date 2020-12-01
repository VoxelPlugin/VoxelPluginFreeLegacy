// Copyright 2020 Phyronnaz

#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelGenerators/VoxelGeneratorCache.h"

FVoxelGeneratorCache& FVoxelGeneratorInit::GetGeneratorCache() const
{
	if (!GeneratorCache)
	{
		GeneratorCache = FVoxelGeneratorCache::Create(*this);
	}
	return *GeneratorCache;
}

void FVoxelGeneratorInit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(MaterialCollection);
	Collector.AddReferencedObject(World);
}