// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelBoxTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelData/VoxelData.h"

void UVoxelBoxTools::SetValueBoxImpl(FVoxelData& Data, const FIntBox& Bounds, FVoxelValue Value)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
	
	Data.Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& OldValue)
	{
		OldValue = Value;
	});
}

template<bool bAdd>
void UVoxelBoxTools::BoxEditImpl(FVoxelData& Data, const FIntBox& Bounds)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
	
	Data.Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
	{
		if (X == Bounds.Min.X || X == Bounds.Max.X - 1 || Y == Bounds.Min.Y || Y == Bounds.Max.Y - 1 || Z == Bounds.Min.Z || Z == Bounds.Max.Z - 1)
		{
			if ((bAdd && Value.IsEmpty()) || (!bAdd && !Value.IsEmpty()))
			{
				Value = FVoxelValue(0.f);
			}
		}
		else
		{
			Value = bAdd ? FVoxelValue::Full() : FVoxelValue::Empty();
		}
	});
}

template VOXEL_API void UVoxelBoxTools::BoxEditImpl<false>(FVoxelData& Data, const FIntBox& Bounds);
template VOXEL_API void UVoxelBoxTools::BoxEditImpl<true>(FVoxelData& Data, const FIntBox& Bounds);

void UVoxelBoxTools::SetMaterialBoxImpl(FVoxelData& Data, const FIntBox& Bounds, const FVoxelPaintMaterial& PaintMaterial)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
	
	Data.Set<FVoxelMaterial>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelMaterial& Material)
	{
		const float Strength = 1.f;
		PaintMaterial.ApplyToMaterial(Material, Strength);
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelBoxTools::SetValueBox(AVoxelWorld* World, FIntBox Bounds, float Value)
{
	VOXEL_TOOL_HELPER(Write, UpdateRender, NO_PREFIX, SetValueBoxImpl(Data, Bounds, FVoxelValue(Value)));
}

void UVoxelBoxTools::AddBox(AVoxelWorld* World, FIntBox Bounds)
{
	VOXEL_TOOL_HELPER(Write, UpdateRender, NO_PREFIX, BoxEditImpl<true>(Data, Bounds));
}

void UVoxelBoxTools::RemoveBox(AVoxelWorld* World, FIntBox Bounds)
{
	VOXEL_TOOL_HELPER(Write, UpdateRender, NO_PREFIX, BoxEditImpl<false>(Data, Bounds));
}

void UVoxelBoxTools::SetMaterialBox(AVoxelWorld* World, FIntBox Bounds, FVoxelPaintMaterial PaintMaterial)
{	
	VOXEL_TOOL_HELPER(Write, UpdateRender, NO_PREFIX, SetMaterialBoxImpl(Data, Bounds, PaintMaterial));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelBoxTools::SetValueBoxAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FIntBox Bounds,
	float Value,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, NO_PREFIX, SetValueBoxImpl(Data, Bounds, FVoxelValue(Value)));
}

void UVoxelBoxTools::AddBoxAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FIntBox Bounds,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, NO_PREFIX, BoxEditImpl<true>(Data, Bounds));
}

void UVoxelBoxTools::RemoveBoxAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FIntBox Bounds,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, NO_PREFIX, BoxEditImpl<false>(Data, Bounds));
}

void UVoxelBoxTools::SetMaterialBoxAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FIntBox Bounds,
	FVoxelPaintMaterial PaintMaterial,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, NO_PREFIX, SetMaterialBoxImpl(Data, Bounds, PaintMaterial));
}