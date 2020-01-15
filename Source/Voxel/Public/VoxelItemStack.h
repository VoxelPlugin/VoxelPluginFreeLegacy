// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRange.h"

class FVoxelPlaceableItemHolder;
class FVoxelWorldGeneratorInstance;

struct VOXEL_API FVoxelItemStack
{
public:
	const FVoxelPlaceableItemHolder& ItemHolder;
	const FVoxelWorldGeneratorInstance* WorldGenerator;
	const int32 Depth; // Index in VoxelAssetItem array, -1 if world generator
	void* const CustomData; // Use this to send custom data to a generator

	explicit FVoxelItemStack(const FVoxelPlaceableItemHolder& ItemHolder, void* CustomData = nullptr)
		: ItemHolder(ItemHolder)
		, WorldGenerator(nullptr)
		, Depth(-1)
		, CustomData(CustomData)
	{
	}
	FVoxelItemStack(const FVoxelPlaceableItemHolder& ItemHolder, const FVoxelWorldGeneratorInstance& WorldGenerator, int32 Depth, void* CustomData = nullptr)
		: ItemHolder(ItemHolder)
		, WorldGenerator(&WorldGenerator)
		, Depth(Depth)
		, CustomData(CustomData)
	{
	}

	static FVoxelItemStack Empty;

	template<typename T>
	T Get(v_flt X, v_flt Y, v_flt Z, int32 LOD) const;
	TVoxelRange<v_flt> GetValueRange(const FIntBox& Bounds, int32 LOD) const;
	
	template<typename T>
	T GetCustomOutput(T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD) const;
	template<typename T>
	TVoxelRange<T> GetCustomOutputRange(TVoxelRange<T> DefaultValue, FName Name, const FIntBox& Bounds, int32 LOD) const;

	template<typename ...TArgs>
	inline FVoxelItemStack GetNextStack(TArgs... Args) const
	{
		return { ItemHolder, *WorldGenerator, GetNextDepth(Args...), CustomData };
	}
	template<typename T>
	inline FVoxelItemStack WithCustomData(T* InCustomData) const
	{
		return { ItemHolder, *WorldGenerator, Depth, const_cast<void*>(reinterpret_cast<const void*>(InCustomData)) };
	}
	inline bool IsEmpty() const
	{
		return Depth == -1;
	}
	inline bool IsValid() const
	{
		return Depth >= -1;
	}

private:
	template<typename ...TArgs>
	int32 GetNextDepth(TArgs... Args) const;
};