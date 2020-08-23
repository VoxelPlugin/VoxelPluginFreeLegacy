// Copyright 2020 Phyronnaz

#include "VoxelPlaceableItems/VoxelPlaceableItem.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorInit.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorInstance.h"
#include "VoxelMessages.h"
#include "VoxelObjectArchive.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelPlaceableItemsPointers);

#define Macro(X) DEFINE_STAT(STAT_Num## X ## Pointers);
FOREACH_VOXEL_ASSET_ITEM(Macro);
#undef Macro

void FVoxelPlaceableItemsUtilities::SerializeItems(
	FVoxelObjectArchive& Ar, 
	const FVoxelPlaceableItemLoadInfo& LoadInfo, 
	TArray<FVoxelAssetItem>& AssetItems)
{
	int32 Version = FVoxelPlaceableItemVersion::LatestVersion;
	Ar << Version;

	int32 NumAssetItems = AssetItems.Num();
	Ar << NumAssetItems;

	if (Ar.IsSaving())
	{
		// Might not be in game thread!
		for (auto& Item : AssetItems)
		{
			TSoftObjectPtr<UVoxelWorldGenerator> WorldGenerator = Item.WorldGenerator->Object;
			if (!ensure(!WorldGenerator.IsNull()))
			{
				LOG_VOXEL(Error, TEXT("Invalid Object pointer on a world generator instance of class %s"), *Item.WorldGenerator->Class->GetName());
			}

			Ar << WorldGenerator;
			Ar << Item.Bounds;
			Ar << Item.LocalToWorld;
			Ar << Item.Priority;
		}
	}
	else
	{
		check(IsInGameThread());
		AssetItems.Empty(NumAssetItems);
		for (int32 Index = 0; Index < NumAssetItems; Index++)
		{
			TSoftObjectPtr<UVoxelTransformableWorldGenerator> WorldGenerator;
			FVoxelIntBox Bounds;
			FTransform LocalToWorld;
			int32 Priority = 0;
			
			Ar << WorldGenerator;
			Ar << Bounds;
			Ar << LocalToWorld;
			Ar << Priority;

			auto* LoadedWorldGenerator = WorldGenerator.LoadSynchronous();
			
			if (!ensure(LoadedWorldGenerator))
			{
				LOG_VOXEL(Error, TEXT("Failed to load %s as VoxelTransformableWorldGenerator"), *WorldGenerator.ToString());
				continue;
			}

			const auto Instance = LoadedWorldGenerator->GetTransformableInstance();
			Instance->Init(ensure(LoadInfo.WorldGeneratorInit) ? *LoadInfo.WorldGeneratorInit : FVoxelWorldGeneratorInit());
			AssetItems.Emplace(FVoxelAssetItem{ Instance, Bounds, LocalToWorld, Priority });
		}
	}
}