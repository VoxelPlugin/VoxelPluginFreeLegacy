// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_VoxelBase.h"
#include "VoxelRender/VoxelMaterialCollection.h"

class FAssetTypeActions_VoxelBasicMaterialCollection : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;

	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_VoxelBasicMaterialCollection", "Voxel Basic Material Collection"); }
	virtual FColor GetTypeColor() const override { return FColor(0, 192, 0); }
	virtual UClass* GetSupportedClass() const override { return UVoxelBasicMaterialCollection::StaticClass(); }
};

class FAssetTypeActions_VoxelMaterialCollection : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;

	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_VoxelMaterialCollection", "Voxel Material Collection"); }
	virtual FColor GetTypeColor() const override { return FColor(0, 192, 0); }
	virtual UClass* GetSupportedClass() const override { return UVoxelMaterialCollection::StaticClass(); }
};