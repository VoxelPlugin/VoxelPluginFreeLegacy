// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_VoxelBase.h"
#include "VoxelSpawners/VoxelSpawnerConfig.h"

class FAssetTypeActions_VoxelSpawnerConfig : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Spawner Config"); }
	virtual FColor GetTypeColor() const override { return FColor(128, 255, 128); }
	virtual UClass* GetSupportedClass() const override { return UVoxelSpawnerConfig::StaticClass(); }
};