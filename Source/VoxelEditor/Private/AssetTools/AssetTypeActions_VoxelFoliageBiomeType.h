// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_VoxelBase.h"
#include "VoxelFoliage/VoxelFoliageBiome.h"

class FAssetTypeActions_VoxelFoliageBiomeType : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Foliage Biome Type"); }
	virtual FColor GetTypeColor() const override { return FColor(128, 255, 128); }
	virtual UClass* GetSupportedClass() const override { return UVoxelFoliageBiomeType::StaticClass(); }
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;
};