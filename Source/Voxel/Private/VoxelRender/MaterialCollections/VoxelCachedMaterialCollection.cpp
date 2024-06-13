// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelRender/MaterialCollections/VoxelCachedMaterialCollection.h"

UMaterialInterface* UVoxelCachedMaterialCollection::GetVoxelMaterial(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const
{
	const FVoxelMaterialIndices Key{ Indices };
	TObjectPtr<UMaterialInterface>& Material = CachedMaterials.FindOrAdd(Key);
	if (!Material)
	{
		// Note: if this errors out and return nullptr, we want to call it again next time
		Material = GetVoxelMaterial_NotCached(Indices, UniqueIdForErrors);
	}
	return Material;
}

void UVoxelCachedMaterialCollection::InitializeCollection()
{
	// Make sure to apply changes
	CachedMaterials.Empty();
}