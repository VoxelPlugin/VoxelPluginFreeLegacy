// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelGenerators/VoxelGeneratorCache.h"
#include "VoxelGenerators/VoxelGeneratorInstance.h"
#include "VoxelGenerators/VoxelGeneratorTools.h"

UVoxelGeneratorInstanceWrapper* UVoxelGeneratorCache::MakeGeneratorInstance(FVoxelGeneratorPicker Picker) const
{
	TObjectPtr<UVoxelGeneratorInstanceWrapper>& Instance = Cache.FindOrAdd(Picker);
	if (!Instance)
	{
		Instance = UVoxelGeneratorTools::MakeGeneratorInstance(Picker, GeneratorInit);
	}
	return Instance;
}

UVoxelTransformableGeneratorInstanceWrapper* UVoxelGeneratorCache::MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker) const
{
	TObjectPtr<UVoxelTransformableGeneratorInstanceWrapper>& Instance = TransformableCache.FindOrAdd(Picker);
	if (!Instance)
	{
		Instance = UVoxelGeneratorTools::MakeTransformableGeneratorInstance(Picker, GeneratorInit);
	}
	return Instance;
}