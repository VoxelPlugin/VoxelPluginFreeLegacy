// Copyright 2020 Phyronnaz

#include "VoxelPlaceableItems/VoxelPlaceableItemManager.h"
#include "VoxelDebug/VoxelDebugUtilities.h"
#include "VoxelDebug/VoxelLineBatchComponent.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorCache.h"
#include "VoxelData/VoxelDataIncludes.h"
#include "VoxelWorld.h"
#include "VoxelMessages.h"

void UVoxelPlaceableItemManager::AddDataItem(FVoxelDataItemConstructionInfo Info)
{
	if (!Info.Generator)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Generator is null!"));
		return;
	}
	if (!Info.Bounds.IsValid())
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Bounds are invalid! ") + Info.Bounds.ToString());
		return;
	}
	DataItemInfos.Add(Info);
}

void UVoxelPlaceableItemManager::DrawDebugLine(FVector Start, FVector End, FLinearColor Color)
{
	DebugLines.Add({ Start, End, Color });
}

void UVoxelPlaceableItemManager::DrawDebugPoint(FVector Position, FLinearColor Color)
{
	DebugPoints.Add({ Position, Color });
}

void UVoxelPlaceableItemManager::Generate()
{
	OnGenerate();
}

void UVoxelPlaceableItemManager::Clear()
{
	DataItemInfos.Reset();
	DebugLines.Reset();
	DebugPoints.Reset();

	OnClear();
}

void UVoxelPlaceableItemManager::ApplyToData(
	FVoxelData& Data,
	const UVoxelWorldGeneratorCache& Cache,
	TMap<FVoxelDataItemConstructionInfo, FVoxelDataItemPtr>* OutItems)
{
	VOXEL_FUNCTION_COUNTER();
	
	for (auto& Info : DataItemInfos)
	{
		if (!ensure(Info.Generator))
		{
			continue;
		}
		
		const auto Instance = Cache.CreateWorldGeneratorInstance(*Info.Generator);

		FVoxelWriteScopeLock Lock(Data, Info.Bounds, FUNCTION_FNAME); // TODO No lock on start
		const auto ItemPtr = Data.AddItem<FVoxelDataItem>(Instance, Info.Bounds, TArray<v_flt>(Info.Parameters), uint32(Info.Mask));
		ensure(ItemPtr.IsValid());
		
		if (OutItems)
		{
			OutItems->Add(Info, ItemPtr);
		}
	}
}

void UVoxelPlaceableItemManager::DrawDebug(const IVoxelWorldInterface& VoxelWorldInterface, UVoxelLineBatchComponent& LineBatchComponent)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!bEnableDebug)
	{
		return;
	}

	const float Lifetime = 100000;

	const auto GetPosition = [&](const FVector& Position)
	{
		return LineBatchComponent.GetComponentTransform().InverseTransformPosition(VoxelWorldInterface.LocalToGlobalFloat(Position));
	};
	
	for (auto& Line : DebugLines)
	{
		LineBatchComponent.BatchedLines.Add(FBatchedLine(
			GetPosition(Line.Start),
			GetPosition(Line.End),
			Line.Color,
			Lifetime,
			0.f,
			0));
	}
	
	for (auto& Point : DebugPoints)
	{
		LineBatchComponent.BatchedPoints.Add(FBatchedPoint(
			GetPosition(Point.Position),
			Point.Color,
			5.f,
			Lifetime,
			0));
	}
	LineBatchComponent.MarkRenderStateDirty();

	if (bDebugBounds)
	{
		for (auto& Info : DataItemInfos)
		{
			UVoxelDebugUtilities::DrawDebugIntBox(VoxelWorldInterface, LineBatchComponent, FTransform(), Info.Bounds, Lifetime);
		}
	}
}