// Copyright 2021 Phyronnaz

#include "VoxelRender/MaterialCollections/VoxelMaterialCollectionBase.h"
#include "Materials/MaterialInterface.h"

FName FVoxelMaterialCollectionMaterialInfo::GetName() const
{
	if (!NameOverride.IsNone())
	{
		return NameOverride;
	}
	if (Material.IsValid())
	{
		return Material->GetFName();
	}
	return {};
}

UMaterialInterface* UVoxelMaterialCollectionBase::GetIndexMaterial(uint8 Index) const
{
	for (const FVoxelMaterialCollectionMaterialInfo& MaterialInfo : GetMaterials())
	{
		if (MaterialInfo.Index == Index)
		{
			return MaterialInfo.Material.Get();
		}
	}
	return nullptr;
}

int32 UVoxelMaterialCollectionBase::GetMaterialIndex(FName Name) const
{
	for (const FVoxelMaterialCollectionMaterialInfo& MaterialInfo : GetMaterials())
	{
		if (MaterialInfo.GetName() == Name)
		{
			return MaterialInfo.Index;
		}
	}
	return -1;
}