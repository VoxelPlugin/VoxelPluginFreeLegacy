// Copyright 2020 Phyronnaz

#include "VoxelRender/MaterialCollections/VoxelCachedMaterialCollection.h"

UMaterialInterface* UVoxelCachedMaterialCollection::GetVoxelMaterial(const FVoxelMaterialIndices& Indices, bool bTessellation, uint64 UniqueIdForErrors) const
{
	const FVoxelCachedMaterialCollectionKey Key{ Indices, bTessellation };
	auto*& Material = CachedMaterials.FindOrAdd(Key);
	if (!Material)
	{
		// Note: if this errors out and return nullptr, we want to call it again next time
		Material = GetVoxelMaterial_NotCached(Indices, bTessellation, UniqueIdForErrors);
	}
	return Material;
}

void UVoxelCachedMaterialCollection::ClearCache()
{
	CachedMaterials.Empty();
}