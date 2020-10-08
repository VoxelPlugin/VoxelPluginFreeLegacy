// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelPlaceableItems/VoxelPlaceableItemManager.h"
#include "VoxelPlaceableItemActorHelper.generated.h"

class AVoxelWorld;
class AVoxelDataItemActor;

UCLASS(Within=VoxelWorld)
class VOXEL_API UVoxelPlaceableItemActorHelper : public UObject
{
	GENERATED_BODY()

public:
	void Initialize();

	AVoxelWorld& GetVoxelWorld() const;

private:
	using FItemInfo = FVoxelDataItemConstructionInfo;
	using FItemPtr = TVoxelWeakPtr<const TVoxelDataItemWrapper<FVoxelDataItem>>;
	
	struct FActorData
	{
		TMap<FItemInfo, FItemPtr> Items;
	};
	TMap<TWeakObjectPtr<AVoxelDataItemActor>, FActorData> ActorsData;
	
	void AddActor(AVoxelDataItemActor& Actor);
	
	void OnActorSpawned(AActor* Actor);
	void OnActorUpdated(TWeakObjectPtr<AVoxelDataItemActor> Actor);
};