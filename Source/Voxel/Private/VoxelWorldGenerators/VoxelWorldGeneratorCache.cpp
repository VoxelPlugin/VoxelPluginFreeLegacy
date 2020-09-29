// Copyright 2020 Phyronnaz

#include "VoxelWorldGenerators/VoxelWorldGeneratorCache.h"
#include "VoxelWorldGenerators/VoxelWorldGenerator.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorInstance.h"

UVoxelWorldGenerator* UVoxelWorldGeneratorCache::FindOrCreateWorldGenerator(FVoxelWorldGeneratorCacheKey Key) const
{
	UVoxelWorldGenerator*& Object = ObjectCache.FindOrAdd(Key);
	if (!Object)
	{
		if (Key.Picker.IsClass())
		{
			Object = NewObject<UVoxelWorldGenerator>(const_cast<UVoxelWorldGeneratorCache*>(this), Key.Picker.Class);
		}
		else
		{
			// TODO
			ensure(false);
		}
	}
	return Object;
}

TVoxelSharedRef<FVoxelWorldGeneratorInstance> UVoxelWorldGeneratorCache::CreateWorldGeneratorInstance(UVoxelWorldGenerator& Generator) const
{
	auto& Instance = InstanceCache.FindOrAdd(&Generator);
	if (!Instance.IsValid())
	{
		Instance = Generator.GetInstance();
		Instance->Init(WorldGeneratorInit);
	}
	return Instance.ToSharedRef();
}
