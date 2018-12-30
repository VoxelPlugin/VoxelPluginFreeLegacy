// Copyright 2018 Phyronnaz

#include "VoxelMaterialCollection.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "Materials/MaterialInstanceDynamic.h"

UMaterialInterface* FVoxelMaterialCollectionGenerated::GetVoxelMaterial(const FVoxelBlendedMaterial& Index)
{
	switch (Index.Kind)
	{
	case FVoxelBlendedMaterial::Single:
	{
		if (!GeneratedSingleMaterials)
		{
			return nullptr;
		}
		UMaterialInterface** Result = GeneratedSingleMaterials->Map.Find(Index.Index0);
		return Result ? *Result : nullptr;
	}
	case FVoxelBlendedMaterial::Double:
	{
		if (!GeneratedDoubleMaterials)
		{
			return nullptr;
		}
		UMaterialInterface** Result = GeneratedDoubleMaterials->Map.Find(FVoxelMaterialCollectionDoubleIndex(Index.Index0, Index.Index1));
		return Result ? *Result : nullptr;
	}
	case FVoxelBlendedMaterial::Triple:
	{
		if (!GeneratedTripleMaterials)
		{
			return nullptr;
		}
		UMaterialInterface** Result = GeneratedTripleMaterials->Map.Find(FVoxelMaterialCollectionTripleIndex(Index.Index0, Index.Index1, Index.Index2));
		return Result ? *Result : nullptr;
	}
	default:
	{
		check(false);
		return nullptr;
	}
	}
}

UMaterialInterface* UVoxelMaterialCollection::GetVoxelMaterial(const FVoxelBlendedMaterial& Index)
{
	return GeneratedMaterials.GetVoxelMaterial(Index);
}

UMaterialInterface* UVoxelMaterialCollection::GetVoxelMaterialWithTessellation(const FVoxelBlendedMaterial& Index)
{
	return GeneratedMaterialsTess.GetVoxelMaterial(Index);
}
