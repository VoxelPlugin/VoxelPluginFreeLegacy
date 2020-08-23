// Copyright 2020 Phyronnaz

#pragma once

#include "VoxelTools/Impl/VoxelBoxToolsImpl.h"
#include "VoxelTools/Impl/VoxelToolsBaseImpl.inl"

#define VOXEL_BOX_TOOL_IMPL() VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());

template<bool bAdd, typename TData>
void FVoxelBoxToolsImpl::BoxEdit(TData& Data, const FVoxelIntBox& Bounds)
{
	VOXEL_BOX_TOOL_IMPL();

	Data.template Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
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

template<typename TData>
void FVoxelBoxToolsImpl::SetValueBox(TData& Data, const FVoxelIntBox& Bounds, FVoxelValue Value)
{
	VOXEL_BOX_TOOL_IMPL();
	
	Data.template Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& OldValue)
	{
		OldValue = Value;
	});
}

template<typename TData>
void FVoxelBoxToolsImpl::SetMaterialBox(TData& Data, const FVoxelIntBox& Bounds, const FVoxelPaintMaterial& PaintMaterial)
{
	VOXEL_BOX_TOOL_IMPL();
	
	Data.template Set<FVoxelMaterial>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelMaterial& Material)
	{
		PaintMaterial.ApplyToMaterial(Material, 1.f);
	});
}