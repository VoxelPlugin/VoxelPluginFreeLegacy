// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_VoxelBase.h"
#include "VoxelRender/MaterialCollections/VoxelBasicMaterialCollection.h"
#include "VoxelRender/MaterialCollections/VoxelLandscapeMaterialCollection.h"
#include "VoxelRender/MaterialCollections/VoxelInstancedMaterialCollection.h"

class FAssetTypeActions_VoxelBasicMaterialCollection : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Basic Material Collection"); }
	virtual FColor GetTypeColor() const override { return FColor(0, 192, 0); }
	virtual UClass* GetSupportedClass() const override { return UVoxelBasicMaterialCollection::StaticClass(); }
};

class FAssetTypeActions_VoxelInstancedMaterialCollectionTemplates : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Instanced Material Collection Templates"); }
	virtual FColor GetTypeColor() const override { return FColor(0, 192, 0); }
	virtual UClass* GetSupportedClass() const override { return UVoxelInstancedMaterialCollectionTemplates::StaticClass(); }
};

class FAssetTypeActions_VoxelInstancedMaterialCollection : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Instanced Material Collection"); }
	virtual FColor GetTypeColor() const override { return FColor(0, 192, 0); }
	virtual UClass* GetSupportedClass() const override { return UVoxelInstancedMaterialCollection::StaticClass(); }
};

class FAssetTypeActions_VoxelInstancedMaterialCollectionInstance : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Instanced Material Collection Instance"); }
	virtual FColor GetTypeColor() const override { return FColor(0, 192, 0); }
	virtual UClass* GetSupportedClass() const override { return UVoxelInstancedMaterialCollectionInstance::StaticClass(); }
};

class FAssetTypeActions_VoxelLandscapeMaterialCollection : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Landscape Material Collection"); }
	virtual FColor GetTypeColor() const override { return FColor(0, 192, 0); }
	virtual UClass* GetSupportedClass() const override { return UVoxelLandscapeMaterialCollection::StaticClass(); }
};