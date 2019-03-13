// Copyright 2019 Phyronnaz

#include "VoxelTools/VoxelBoxTools.h"
#include "VoxelTools/VoxelToolsHelpers.h"
#include "VoxelTools/VoxelLatentActionHelpers.h"
#include "VoxelRender/IVoxelLODManager.h"

DECLARE_CYCLE_STAT(TEXT("UVoxelBoxTools::SetValueBox"),    STAT_UVoxelBoxTools_SetValueBox, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelBoxTools::AddBox"),         STAT_UVoxelBoxTools_AddBox, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelBoxTools::RemoveBox"),      STAT_UVoxelBoxTools_RemoveBox, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelBoxTools::SetMaterialBox"), STAT_UVoxelBoxTools_SetMaterialBox, STATGROUP_Voxel);

void SetValueBoxImpl(FVoxelData& Data, const FIntBox& Bounds, float Value)
{
	Data.SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& OldValue)
	{
		OldValue = Value;
	});
}

template<bool bAdd>
void BoxEditImpl(FVoxelData& Data, const FIntBox& Bounds)
{
	Data.SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& Value)
	{
		if (X == Bounds.Min.X || X == Bounds.Max.X - 1 || Y == Bounds.Min.Y || Y == Bounds.Max.Y - 1 || Z == Bounds.Min.Z || Z == Bounds.Max.Z - 1)
		{
			if ((bAdd && Value.IsEmpty()) || (!bAdd && !Value.IsEmpty()))
			{
				Value = 0;
			}
		}
		else
		{
			Value = bAdd ? FVoxelValue::Full : FVoxelValue::Empty;
		}
	});
}

void SetMaterialBoxImpl(FVoxelData& Data, const FIntBox& Bounds, const FVoxelPaintMaterial& PaintMaterial)
{
	Data.SetValueOrMaterialLambda<FVoxelMaterial>(Bounds, [&](int X, int Y, int Z, FVoxelMaterial& Material)
	{
		PaintMaterial.ApplyToMaterial(Material);
	});
}

///////////////////////////////////////////////////////////////////////////////

bool UVoxelBoxTools::SetValueBox(AVoxelWorld* World, const FIntBox& Bounds, float Value, bool bUpdateRender, bool bAllowFailure)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelBoxTools_SetValueBox);
	CHECK_VOXELWORLD_IS_CREATED();

	return FVoxelToolsHelpers::EditToolsHelper<EVoxelLockType::ReadWrite>(__FUNCTION__, World, Bounds, bUpdateRender, bAllowFailure, [&](FVoxelData& Data)
	{
		SetValueBoxImpl(Data, Bounds, Value);
	});
}

bool UVoxelBoxTools::AddBox(AVoxelWorld* World, const FIntBox& Bounds, bool bUpdateRender, bool bAllowFailure)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelBoxTools_AddBox);
	CHECK_VOXELWORLD_IS_CREATED();
	
	return FVoxelToolsHelpers::EditToolsHelper<EVoxelLockType::ReadWrite>(__FUNCTION__, World, Bounds, bUpdateRender, bAllowFailure, [&](FVoxelData& Data)
	{
		BoxEditImpl<true>(Data, Bounds);
	});
}

bool UVoxelBoxTools::RemoveBox(AVoxelWorld* World, const FIntBox& Bounds, bool bUpdateRender, bool bAllowFailure)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelBoxTools_RemoveBox);
	CHECK_VOXELWORLD_IS_CREATED();
	
	return FVoxelToolsHelpers::EditToolsHelper<EVoxelLockType::ReadWrite>(__FUNCTION__, World, Bounds, bUpdateRender, bAllowFailure, [&](FVoxelData& Data)
	{
		BoxEditImpl<false>(Data, Bounds);
	});
}

bool UVoxelBoxTools::SetMaterialBox(AVoxelWorld* World, const FIntBox& Bounds, FVoxelPaintMaterial PaintMaterial, bool bUpdateRender, bool bAllowFailure)
{	
	SCOPE_CYCLE_COUNTER(STAT_UVoxelBoxTools_SetMaterialBox);
	CHECK_VOXELWORLD_IS_CREATED();
	
	return FVoxelToolsHelpers::EditToolsHelper<EVoxelLockType::ReadWrite>(__FUNCTION__, World, Bounds, bUpdateRender, bAllowFailure, [&](FVoxelData& Data)
	{
		SetMaterialBoxImpl(Data, Bounds, PaintMaterial);
	});
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelBoxTools::SetValueBoxAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	const FIntBox& Bounds,
	float Value,
	bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	FVoxelLatentActionHelpers::AsyncHelper(WorldContextObject, LatentInfo, __FUNCTION__, World, Bounds, bUpdateRender, [Bounds, Value](FVoxelData& Data)
	{
		FVoxelReadWriteScopeLock Lock(Data, Bounds, __FUNCTION__);
		SetValueBoxImpl(Data, Bounds, Value);
	});
}

void UVoxelBoxTools::AddBoxAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	const FIntBox& Bounds,
	bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	FVoxelLatentActionHelpers::AsyncHelper(WorldContextObject, LatentInfo, __FUNCTION__, World, Bounds, bUpdateRender, [Bounds](FVoxelData& Data)
	{
		FVoxelReadWriteScopeLock Lock(Data, Bounds, __FUNCTION__);
		BoxEditImpl<true>(Data, Bounds);
	});
}

void UVoxelBoxTools::RemoveBoxAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	const FIntBox& Bounds,
	bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	FVoxelLatentActionHelpers::AsyncHelper(WorldContextObject, LatentInfo, __FUNCTION__, World, Bounds, bUpdateRender, [Bounds](FVoxelData& Data)
	{
		FVoxelReadWriteScopeLock Lock(Data, Bounds, __FUNCTION__);
		BoxEditImpl<false>(Data, Bounds);
	});
}

void UVoxelBoxTools::SetMaterialBoxAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	const FIntBox& Bounds,
	FVoxelPaintMaterial PaintMaterial,
	bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	FVoxelLatentActionHelpers::AsyncHelper(WorldContextObject, LatentInfo, __FUNCTION__, World, Bounds, bUpdateRender, [Bounds, PaintMaterial](FVoxelData& Data)
	{
		FVoxelReadWriteScopeLock Lock(Data, Bounds, __FUNCTION__);
		SetMaterialBoxImpl(Data, Bounds, PaintMaterial);
	});
}
