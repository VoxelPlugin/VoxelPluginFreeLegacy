// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRange.h"

class FVoxelPlaceableItemHolder;
class FVoxelGeneratorInstance;

class VOXEL_API FVoxelGeneratorQueryData
{
public:
	static const FVoxelGeneratorQueryData Empty;
	
	FVoxelGeneratorQueryData() = default;
	UE_NONCOPYABLE(FVoxelGeneratorQueryData);

public:
	// The list of data item parameters
	TArrayView<const v_flt> DataItemParameters;
	// Used for foliage
	TFunction<FVoxelMaterial()> GetMaterial;
};

struct VOXEL_API FVoxelItemStack
{
public:
	const FVoxelPlaceableItemHolder& ItemHolder;
	const FVoxelGeneratorInstance* const Generator;
	const int32 Depth; // Index in VoxelAssetItem array, -1 if generator
	const FVoxelGeneratorQueryData& QueryData;

	explicit FVoxelItemStack(const FVoxelPlaceableItemHolder& ItemHolder, const FVoxelGeneratorQueryData& QueryData = FVoxelGeneratorQueryData::Empty)
		: ItemHolder(ItemHolder)
		, Generator(nullptr)
		, Depth(-1)
		, QueryData(QueryData)
	{
	}
	FVoxelItemStack(const FVoxelPlaceableItemHolder& ItemHolder, const FVoxelGeneratorInstance& Generator, int32 Depth, const FVoxelGeneratorQueryData& QueryData = FVoxelGeneratorQueryData::Empty)
		: ItemHolder(ItemHolder)
		, Generator(&Generator)
		, Depth(Depth)
		, QueryData(QueryData)
	{
	}

	static const FVoxelItemStack Empty;

	VOXEL_DEPRECATED(1.3, "Use QueryData instead")
	static constexpr const TArray<v_flt>* CustomData = nullptr;

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
		return { ItemHolder, *Generator, GetNextDepth(Args...), QueryData };
	}
	FORCEINLINE FVoxelItemStack WithQueryData(const FVoxelGeneratorQueryData& InQueryData) const
	{
		return { ItemHolder, *Generator, Depth, InQueryData };
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