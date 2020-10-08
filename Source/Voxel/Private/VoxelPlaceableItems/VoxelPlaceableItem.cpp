// Copyright 2020 Phyronnaz

#include "VoxelPlaceableItems/VoxelPlaceableItem.h"
#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelGenerators/VoxelGeneratorInstance.h"
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
			TSoftObjectPtr<UVoxelGenerator> Generator = Item.Generator->Object;
			if (!ensure(!Generator.IsNull()))
			{
				LOG_VOXEL(Error, TEXT("Invalid Object pointer on a generator instance of class %s"), *Item.Generator->Class->GetName());
			}

			Ar << Generator;
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
			TSoftObjectPtr<UVoxelTransformableGenerator> Generator;
			FVoxelIntBox Bounds;
			FTransform LocalToWorld;
			int32 Priority = 0;
			
			Ar << Generator;
			Ar << Bounds;
			Ar << LocalToWorld;
			Ar << Priority;

			auto* LoadedGenerator = Generator.LoadSynchronous();
			
			if (!ensure(LoadedGenerator))
			{
				LOG_VOXEL(Error, TEXT("Failed to load %s as VoxelTransformableGenerator"), *Generator.ToString());
				continue;
			}

			const auto Instance = LoadedGenerator->GetTransformableInstance();
			Instance->Init(ensure(LoadInfo.GeneratorInit) ? *LoadInfo.GeneratorInit : FVoxelGeneratorInit());
			AssetItems.Emplace(FVoxelAssetItem{ Instance, Bounds, LocalToWorld, Priority });
		}
	}
}