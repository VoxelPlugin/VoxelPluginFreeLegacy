// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolsBase.h"

template<typename T>
FIntBox GetModifiedVoxelsBounds(const TArray<T>& ModifiedVoxels)
{
	FIntBoxWithValidity Bounds;
	for (auto& ModifiedVoxel : ModifiedVoxels)
	{
		Bounds += ModifiedVoxel.Position;
	}
	return Bounds.IsValid() ? Bounds.GetBox() : FIntBox();
}

FIntBox UVoxelToolsBase::GetModifiedVoxelValuesBounds(const TArray<FModifiedVoxelValue>& ModifiedVoxels)
{
	VOXEL_FUNCTION_COUNTER();
	return GetModifiedVoxelsBounds(ModifiedVoxels);
}

FIntBox UVoxelToolsBase::GetModifiedVoxelMaterialsBounds(const TArray<FModifiedVoxelMaterial>& ModifiedVoxels)
{
	VOXEL_FUNCTION_COUNTER();
	return GetModifiedVoxelsBounds(ModifiedVoxels);
}

FIntBox UVoxelToolsBase::GetModifiedVoxelFoliagesBounds(const TArray<FModifiedVoxelFoliage>& ModifiedVoxels)
{
	VOXEL_FUNCTION_COUNTER();
	return GetModifiedVoxelsBounds(ModifiedVoxels);
}