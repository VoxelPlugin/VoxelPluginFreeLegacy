// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRange.h"

class FVoxelPlaceableItemHolder;
class FVoxelGeneratorInstance;

struct VOXEL_API FVoxelItemStack
{
public:
	const FVoxelPlaceableItemHolder& ItemHolder;
	const FVoxelGeneratorInstance* const Generator;
	const int32 Depth; // Index in VoxelAssetItem array, -1 if generator
	const TArray<v_flt>* const CustomData; // Use this to send custom data to a generator

	explicit FVoxelItemStack(const FVoxelPlaceableItemHolder& ItemHolder, const TArray<v_flt>* CustomData = nullptr)
		: ItemHolder(ItemHolder)
		, Generator(nullptr)
		, Depth(-1)
		, CustomData(CustomData)
	{
	}
	FVoxelItemStack(const FVoxelPlaceableItemHolder& ItemHolder, const FVoxelGeneratorInstance& Generator, int32 Depth, const TArray<v_flt>* CustomData = nullptr)
		: ItemHolder(ItemHolder)
		, Generator(&Generator)
		, Depth(Depth)
		, CustomData(CustomData)
	{
	}

	static FVoxelItemStack Empty;

	template<typename T>
	T Get(v_flt X, v_flt Y, v_flt Z, int32 LOD) const;
	TVoxelRange<v_flt> GetValueRange(const FVoxelIntBox& Bounds, int32 LOD) const;
	
	template<typename T>
	T GetCustomOutput(T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD) const;
	template<typename T>
	TVoxelRange<T> GetCustomOutputRange(TVoxelRange<T> DefaultValue, FName Name, const FVoxelIntBox& Bounds, int32 LOD) const;

	template<typename ...TArgs>
	FORCEINLINE FVoxelItemStack GetNextStack(TArgs... Args) const
	{
		return { ItemHolder, *Generator, GetNextDepth(Args...), CustomData };
	}
	FORCEINLINE FVoxelItemStack WithCustomData(const TArray<v_flt>* InCustomData) const
	{
		return { ItemHolder, *Generator, Depth, InCustomData };
	}
	FORCEINLINE bool IsEmpty() const
	{
		return Depth == -1;
	}
	FORCEINLINE bool IsValid() const
	{
		return Depth >= -1;
	}

private:
	template<typename ...TArgs>
	int32 GetNextDepth(TArgs... Args) const;
};