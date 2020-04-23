// Copyright 2020 Phyronnaz

#include "VoxelPlaceableItems/VoxelPlaceableItem.h"
#include "VoxelMessages.h"

const FVoxelPlaceableItemHolder FVoxelPlaceableItemHolder::Empty;

FVoxelPlaceableItemLoader* FVoxelPlaceableItemLoader::GetLoader(uint8 ItemId)
{
	auto& Loaders = GetStaticLoaders();
	checkf(Loaders.IsValidIndex(ItemId) && Loaders[ItemId], TEXT("Item with Id=%d isn't constructed. You need to create a FVoxelPlaceableItemLoader(YourItemId);"), ItemId);
	return Loaders[ItemId];
}

TArray<FVoxelPlaceableItemLoader*>& FVoxelPlaceableItemLoader::GetStaticLoaders()
{
	static TArray<FVoxelPlaceableItemLoader*> Loaders;
	return Loaders;
}

FArchive& SerializeVoxelItem(FArchive& Ar, const AVoxelWorld* VoxelWorld, TVoxelSharedPtr<FVoxelPlaceableItem>& Item)
{
	uint8 ItemId = Ar.IsSaving() ? Item->ItemId : 0;
	Ar << ItemId;
	if (ItemId == 0)
	{
		ensureAlways(Ar.IsLoading());
		FVoxelMessages::Error("Invalid ItemId: Save is corrupted", (UObject*)VoxelWorld);
		Ar.SetError();
		return Ar;
	}

	if (Ar.IsLoading())
	{
		check(VoxelWorld);
		Item = FVoxelPlaceableItemLoader::GetLoader(ItemId)->Load(Ar, VoxelWorld);
	}
	else
	{
		Item->Save(Ar);
	}
	return Ar;
}

FVoxelPlaceableItemLoader::FVoxelPlaceableItemLoader(uint8 ItemId)
{
	auto& Loaders = GetStaticLoaders();
	if (Loaders.Num() <= ItemId)
	{
		Loaders.SetNum(ItemId + 1);
	}

	checkf(!Loaders[ItemId], TEXT("Item %d as 2 loaders!"), ItemId);

	Loaders[ItemId] = this;
}