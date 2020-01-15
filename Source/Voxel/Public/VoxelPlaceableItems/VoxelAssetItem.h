// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"

class FVoxelTransformableWorldGeneratorInstance;

class VOXEL_API FVoxelAssetItem : public FVoxelPlaceableItem
{
public:
	FORCEINLINE static int32 StaticId() { return EVoxelPlaceableItemId::Asset; }

	const TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> WorldGenerator;
	const FTransform LocalToWorld;
	const FIntBox WorldBounds;

	FVoxelAssetItem(
		const TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance>& WorldGenerator,
		const FIntBox& WorldBounds, 
		const FTransform& LocalToWorld,
		int32 Priority);

	virtual FString GetDescription() const override;
	virtual void Save(FArchive& Ar) const override;
};

class VOXEL_API FVoxelAssetItemLoader : public FVoxelPlaceableItemLoader
{
public:
	static FVoxelAssetItemLoader AssetLoader;

	FVoxelAssetItemLoader()
		: FVoxelPlaceableItemLoader(FVoxelAssetItem::StaticId())
	{
	}

	virtual TVoxelSharedRef<FVoxelPlaceableItem> Load(FArchive& Ar, const AVoxelWorld* VoxelWorld) const override;
};