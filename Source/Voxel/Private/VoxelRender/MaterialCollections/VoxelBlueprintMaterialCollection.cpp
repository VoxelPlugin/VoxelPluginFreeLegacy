// Copyright 2021 Phyronnaz

#include "VoxelRender/MaterialCollections/VoxelBlueprintMaterialCollection.h"
#include "VoxelRender/VoxelMaterialIndices.h"

void UVoxelBlueprintMaterialCollection::InitializeCollection()
{
	if (!Instance)
	{
		return;
	}

	Instance->InitializeCollection();
}

bool UVoxelBlueprintMaterialCollection::EnableCubicFaces() const
{
	if (!Instance)
	{
		return false;
	}

	return Instance->bEnableCubicFaces;
}

int32 UVoxelBlueprintMaterialCollection::GetMaxMaterialIndices() const
{
	if (!Instance)
	{
		return 1;
	}
	
	return Instance->MaxMaterialsToBlendAtOnce;
}

UMaterialInterface* UVoxelBlueprintMaterialCollection::GetVoxelMaterial_NotCached(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const
{
	if (!Instance)
	{
		return nullptr;
	}

	if (Indices.NumIndices == 1)
	{
		return Instance->GetMaterialForIndex(Indices.SortedIndices[0], EVoxelCubicFace(Indices.CubicFace));
	}
	else
	{
		return Instance->GetMaterialForIndices(Indices.ToArray());
	}
}

TArray<FVoxelMaterialCollectionMaterialInfo> UVoxelBlueprintMaterialCollection::GetMaterials() const
{
	if (!Instance)
	{
		return {};
	}

	return Instance->GetMaterials();
}
