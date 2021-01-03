// Copyright 2021 Phyronnaz

#include "VoxelPlaceableItems/Actors/VoxelPlaceableItemActorHelper.h"
#include "VoxelPlaceableItems/Actors/VoxelDataItemActor.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelDataIncludes.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelFoliage/VoxelFoliageInterface.h"

#include "EngineUtils.h"
#include "TimerManager.h"

void UVoxelPlaceableItemActorHelper::Initialize()
{
	for (TActorIterator<AVoxelDataItemActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AddActor(**ActorItr);
	}

	GetWorld()->AddOnActorSpawnedHandler(MakeWeakObjectPtrDelegate(this, [=](AActor* Actor)
	{
		auto* DataItemActor = Cast<AVoxelDataItemActor>(Actor);
		if (!DataItemActor)
		{
			return;
		}
	
		if (Actor->GetWorld()->IsGameWorld())
		{
			// In games, delay by one frame so that BeginPlay/the construction script have time to run
			Actor->GetWorld()->GetTimerManager().SetTimerForNextTick(MakeWeakObjectPtrDelegate(this, [this, WeakDataItemActor = MakeWeakObjectPtr(DataItemActor)]()
			{
				if (AVoxelDataItemActor* Object = WeakDataItemActor.Get())
				{
					AddActor(*Object);
				}
			}));
		}
		else
		{
			// In the editor, just handle the actor now
			AddActor(*DataItemActor);
		}
	}));
	FWorldDelegates::LevelAddedToWorld.Add(FWorldDelegates::FOnLevelChanged::FDelegate::CreateUObject(this, &UVoxelPlaceableItemActorHelper::OnLevelAddedToWorld));
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
	ensure(!ActorsData.Contains(&Actor));
	ActorsData.Add(&Actor);

	Actor.OnRefresh.AddUObject(this, &UVoxelPlaceableItemActorHelper::OnActorUpdated, MakeWeakObjectPtr(&Actor));

	OnActorUpdated(&Actor);
}

// When the engine streams levels in, the order of initialization of actors is not deterministic, and no spawn nor construct events are
// broadcast for the new actors. Therefore, if a data item actor and a voxel world are in the same streaming level but the
// data item actor is created after the voxel world, both Initialize() and AddOnActorSpawnedHandler() will miss the item actor
// When a new level is streamed in, we need to reparse actors to find the new ones
void UVoxelPlaceableItemActorHelper::OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld)
{
	// Maybe we should filter for actors in our same level, but this is consistent with Initialize()
	if (InWorld != GetWorld())
	{
		return;
	}
	
	for (TActorIterator<AVoxelDataItemActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		auto* Actor = *ActorItr;
		if (!ActorsData.Contains(Actor))
		{
			AddActor(*Actor);
		}
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

	FVoxelData& Data = VoxelWorld.GetSubsystemChecked<FVoxelData>();

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
		VoxelWorld.GetSubsystemChecked<IVoxelLODManager>().UpdateBounds(BoundsToUpdate);

		if (Data.bEnableUndoRedo && !Data.IsCurrentFrameEmpty())
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