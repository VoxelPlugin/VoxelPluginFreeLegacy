// Copyright 2020 Phyronnaz

#include "VoxelGenerators/VoxelGeneratorCache.h"
#include "VoxelGenerators/VoxelGeneratorInstance.h"
#include "VoxelGenerators/VoxelGeneratorTools.h"

UVoxelGeneratorInstanceWrapper* UVoxelGeneratorCache::MakeGeneratorInstance(FVoxelGeneratorPicker Picker) const
{
	auto*& Instance = Cache.FindOrAdd(Picker);
	if (!Instance)
	{
		Instance = UVoxelGeneratorTools::MakeGeneratorInstance(Picker, GeneratorInit);
	}
	return Instance;
}

UVoxelTransformableGeneratorInstanceWrapper* UVoxelGeneratorCache::MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker) const
{
	auto*& Instance = TransformableCache.FindOrAdd(Picker);
	if (!Instance)
	{
		Instance = UVoxelGeneratorTools::MakeTransformableGeneratorInstance(Picker, GeneratorInit);
	}
	return Instance;
}