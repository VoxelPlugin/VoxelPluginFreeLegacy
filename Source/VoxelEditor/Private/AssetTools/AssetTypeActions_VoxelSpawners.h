// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_VoxelBase.h"
#include "VoxelSpawners/VoxelMeshSpawner.h"
#include "VoxelSpawners/VoxelAssetSpawner.h"
#include "VoxelSpawners/VoxelSpawnerGroup.h"

class FAssetTypeActions_VoxelSpawnerBase : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;
	
	virtual FColor GetTypeColor() const override { return FColor(128, 255, 128); }
};

class FAssetTypeActions_VoxelMeshSpawner : public FAssetTypeActions_VoxelSpawnerBase
{
public:
	using FAssetTypeActions_VoxelSpawnerBase::FAssetTypeActions_VoxelSpawnerBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Mesh Spawner"); }
	virtual UClass* GetSupportedClass() const override { return UVoxelMeshSpawner::StaticClass(); }
};

class FAssetTypeActions_VoxelMeshSpawnerGroup : public FAssetTypeActions_VoxelSpawnerBase
{
public:
	using FAssetTypeActions_VoxelSpawnerBase::FAssetTypeActions_VoxelSpawnerBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Mesh Spawner Group"); }
	virtual UClass* GetSupportedClass() const override { return UVoxelMeshSpawnerGroup::StaticClass(); }
};

class FAssetTypeActions_VoxelAssetSpawner : public FAssetTypeActions_VoxelSpawnerBase
{
public:
	using FAssetTypeActions_VoxelSpawnerBase::FAssetTypeActions_VoxelSpawnerBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Asset Spawner"); }
	virtual UClass* GetSupportedClass() const override { return UVoxelAssetSpawner::StaticClass(); }
};

class FAssetTypeActions_VoxelSpawnerGroup : public FAssetTypeActions_VoxelSpawnerBase
{
public:
	using FAssetTypeActions_VoxelSpawnerBase::FAssetTypeActions_VoxelSpawnerBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Spawner Group"); }
	virtual UClass* GetSupportedClass() const override { return UVoxelSpawnerGroup::StaticClass(); }
};