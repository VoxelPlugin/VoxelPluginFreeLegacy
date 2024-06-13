// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelPlaceableItems/Actors/VoxelPlaceableItemActorHelper.h"
#include "VoxelPlaceableItems/Actors/VoxelDataItemActor.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelDataIncludes.h"
#include "VoxelRender/IVoxelLODManager.h"

#include "EngineUtils.h"

void UVoxelPlaceableItemActorHelper::Initialize()
{
	for (TActorIterator<AVoxelDataItemActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AddActor(**ActorItr);
	}

	GetWorld()->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &UVoxelPlaceableItemActorHelper::OnActorSpawned));
}

AVoxelWorld& UVoxelPlaceableItemActorHelper::GetVoxelWorld() const
{
	return *CastChecked<AVoxelWorld>(GetOuter());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelPlaceableItemActorHelper::AddActor(AVoxelDataItemActor& Actor)
{
	AVoxelWorld& VoxelWorld = GetVoxelWorld();
	if (!ensure(VoxelWorld.IsCreated())) return;

	if (!ensure(VoxelWorld.PlaceableItemManager)) return;
	UVoxelPlaceableItemManager& PlaceableItemManager = *VoxelWorld.PlaceableItemManager;

	FActorData& ActorData = ActorsData.FindOrAdd(&Actor);

	PlaceableItemManager.Clear();
	Actor.CallAddItemToWorld(&VoxelWorld);

	PlaceableItemManager.ApplyToData(VoxelWorld.GetData(), &ActorData.Items);
	PlaceableItemManager.DrawDebug(VoxelWorld, VoxelWorld.GetLineBatchComponent());

	Actor.OnRefresh.AddUObject(this, &UVoxelPlaceableItemActorHelper::OnActorUpdated, MakeWeakObjectPtr(&Actor));
}

void UVoxelPlaceableItemActorHelper::OnActorSpawned(AActor* Actor)
{
	if (auto* DataItemActor = Cast<AVoxelDataItemActor>(Actor))
	{
		AddActor(*DataItemActor);
	}
}

void UVoxelPlaceableItemActorHelper::OnActorUpdated(TWeakObjectPtr<AVoxelDataItemActor> Actor)
{
	if (!ensure(Actor.IsValid()) || !ensure(ActorsData.Contains(Actor)))
	{
		return;
	}

	FActorData& ActorData = ActorsData[Actor];

	AVoxelWorld& VoxelWorld = GetVoxelWorld();
	if (!ensure(VoxelWorld.IsCreated()) || !ensure(IsValid(&VoxelWorld))) return;

	if (!ensure(VoxelWorld.PlaceableItemManager)) return;
	UVoxelPlaceableItemManager& PlaceableItemManager = *VoxelWorld.PlaceableItemManager;

	FVoxelData& Data = VoxelWorld.GetData();

	TArray<FVoxelIntBox> BoundsToUpdate;
	{
		// Find which items to add/remove
		TSet<FItemInfo> ItemInfosToAdd;
		TSet<FItemInfo> ItemInfosToRemove;
		{
			PlaceableItemManager.Clear();
			if (!Actor->IsActorBeingDestroyed()) // If we're being destroyed, just remove existing items
			{
				Actor->CallAddItemToWorld(&VoxelWorld);
			}

			TSet<FItemInfo> PreviousItemInfos;
			TSet<FItemInfo> NewItemInfos;

			for (auto& It : ActorData.Items)
			{
				PreviousItemInfos.Add(It.Key);
			}
			NewItemInfos.Append(PlaceableItemManager.GetDataItemInfos());

			ItemInfosToAdd = NewItemInfos.Difference(PreviousItemInfos);
			ItemInfosToRemove = PreviousItemInfos.Difference(NewItemInfos);
		}

		// Remove the items that aren't here anymore
		for (const auto& ItemInfoToRemove : ItemInfosToRemove)
		{
			FItemPtr ItemPtr;
			if (!ensure(ActorData.Items.RemoveAndCopyValue(ItemInfoToRemove, ItemPtr)))
			{
				continue;
			}

			BoundsToUpdate.Add(ItemInfoToRemove.Bounds);

			FVoxelWriteScopeLock Lock(Data, ItemInfoToRemove.Bounds, FUNCTION_FNAME);
			FString Error;
			if (!ensure(Data.RemoveItem(ItemPtr, Error)))
			{
				LOG_VOXEL(Error, TEXT("Failed to remove data item for %s: %s"), *Actor->GetName(), *Error);
			}
		}

		// Add the new ones
		{
			PlaceableItemManager.Clear();

			for (auto& ItemInfoToAdd : ItemInfosToAdd)
			{
				PlaceableItemManager.AddDataItem(ItemInfoToAdd);
			}

			TMap<FItemInfo, FItemPtr> NewItems;
			PlaceableItemManager.ApplyToData(Data, &NewItems);

			for (auto& NewIt : NewItems)
			{
				const FItemInfo& ItemInfo = NewIt.Key;
				const FItemPtr& ItemPtr = NewIt.Value;
				ensure(ItemPtr.IsValid());

				BoundsToUpdate.Add(ItemInfo.Bounds);
				ActorData.Items.Add(ItemInfo, ItemPtr);
			}
		}
	}

	if (BoundsToUpdate.Num() > 0)
	{
		VoxelWorld.GetLODManager().UpdateBounds(BoundsToUpdate);

		if (!Data.IsCurrentFrameEmpty())
		{
			// Save the frame for the eventual asset item merge/remove edits
			// Dummy frame, doesn't really store anything interesting
			Data.SaveFrame(FVoxelIntBox(BoundsToUpdate));

#if WITH_EDITOR
			IVoxelWorldEditor::GetVoxelWorldEditor()->RegisterTransaction(&VoxelWorld, "Applying voxel data item");
#endif
		}

	}

	PlaceableItemManager.DrawDebug(VoxelWorld, VoxelWorld.GetLineBatchComponent());
}