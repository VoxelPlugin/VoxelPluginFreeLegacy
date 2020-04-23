// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_VoxelBase.h"
#include "VoxelData/VoxelSave.h"

class FAssetTypeActions_VoxelWorldSaveObject : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel World Save Object"); }
	virtual FColor GetTypeColor() const override { return FColor(255, 140, 0); }
	virtual UClass* GetSupportedClass() const override { return UVoxelWorldSaveObject::StaticClass(); }
};