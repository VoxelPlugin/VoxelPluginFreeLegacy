// Copyright 2020 Phyronnaz

#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelGenerators/VoxelGeneratorCache.h"

const IVoxelGeneratorCache& FVoxelGeneratorInit::GetGeneratorCache() const
{
	if (auto* Cache = GeneratorCache.Get())
	{
		return *Cache;
	}
	
	if (!GeneratorCacheFallback)
	{
		GeneratorCacheFallback = MakeVoxelShared<FVoxelEmptyGeneratorCache>(*this);
	}
	return *GeneratorCacheFallback;
}