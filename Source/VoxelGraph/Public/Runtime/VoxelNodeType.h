// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphGlobals.h"
#include "VoxelMaterial.h"
#include "VoxelRange.h"

using FVoxelGraphSeed = int32;

struct FVoxelNodeType
{
	static constexpr int32 VoxelNodeTypeSize = FMath::Max(sizeof(v_flt), FMath::Max(sizeof(FVoxelMaterial), sizeof(int32)));

	FVoxelNodeType() = default;

	template<typename T>
	FORCEINLINE T& Get()
	{
		static_assert(sizeof(T) <= VoxelNodeTypeSize, "");
		static_assert(TOr<TIsSame<T, v_flt>, TIsSame<T, int32>, TIsSame<T, bool>, TIsSame<T, FVoxelMaterial>, TIsSame<T, FColor>>::Value, "");
		return *reinterpret_cast<T*>(Data);
	}
	template<typename T>
	FORCEINLINE const T& Get() const
	{
		static_assert(sizeof(T) <= VoxelNodeTypeSize, "");
		static_assert(TOr<TIsSame<T, v_flt>, TIsSame<T, int32>, TIsSame<T, bool>, TIsSame<T, FVoxelMaterial>, TIsSame<T, FColor>>::Value, "");
		return *reinterpret_cast<const T*>(Data);
	}

private:
	static_assert(sizeof(v_flt) <= VoxelNodeTypeSize, "");
	static_assert(sizeof(int32) <= VoxelNodeTypeSize, "");
	static_assert(sizeof(bool) <= VoxelNodeTypeSize, "");
	static_assert(sizeof(FVoxelMaterial) <= VoxelNodeTypeSize, "");
	static_assert(sizeof(FColor) <= VoxelNodeTypeSize, "");

	uint8 Data[VoxelNodeTypeSize];
};

struct FVoxelNodeRangeType
{
	static constexpr int32 VoxelNodeRangeTypeSize = sizeof(TVoxelRange<v_flt>);
	
	FVoxelNodeRangeType() = default;

	template<typename T>
	auto& Get();
	template<typename T>
	const auto& Get() const;

private:
	static_assert(sizeof(TVoxelRange<v_flt>) <= VoxelNodeRangeTypeSize, "");
	static_assert(sizeof(TVoxelRange<int32>) <= VoxelNodeRangeTypeSize, "");
	static_assert(sizeof(FVoxelBoolRange) <= VoxelNodeRangeTypeSize, "");
	static_assert(sizeof(FVoxelMaterialRange) <= VoxelNodeRangeTypeSize, "");
	static_assert(sizeof(FVoxelColorRange) <= VoxelNodeRangeTypeSize, "");

	uint8 Data[VoxelNodeRangeTypeSize];
};

#define DEFINE_GET(T1, T2) \
	template<> FORCEINLINE auto& FVoxelNodeRangeType::Get<T1>() { static_assert(sizeof(T2) <= VoxelNodeRangeTypeSize, ""); return *reinterpret_cast<T2*>(Data); } \
	template<> FORCEINLINE const auto& FVoxelNodeRangeType::Get<T1>() const { static_assert(sizeof(T2) <= VoxelNodeRangeTypeSize, ""); return *reinterpret_cast<const T2*>(Data); }

	DEFINE_GET(v_flt, TVoxelRange<v_flt>);
	DEFINE_GET(int32, TVoxelRange<int32>);
	DEFINE_GET(bool, FVoxelBoolRange);
	DEFINE_GET(FVoxelMaterial, FVoxelMaterialRange);
	DEFINE_GET(FColor, FVoxelColorRange);
#undef DEFINE_GET

template<typename T>
struct TVoxelNodeBuffer
{
	T* RESTRICT const Buffer;

	TVoxelNodeBuffer(T* Buffer)
		: Buffer(Buffer)
	{
	}

	FORCEINLINE T& operator[](int32 Index)
	{
		checkVoxelGraph(0 <= Index && Index < MAX_VOXELNODE_PINS);
		return Buffer[Index];
	}
};

struct FVoxelNodeInputBuffer : TVoxelNodeBuffer<const FVoxelNodeType>
{
	using TVoxelNodeBuffer::TVoxelNodeBuffer;
};
struct FVoxelNodeOutputBuffer : TVoxelNodeBuffer<FVoxelNodeType>
{
	using TVoxelNodeBuffer::TVoxelNodeBuffer;
};

struct FVoxelNodeInputRangeBuffer : TVoxelNodeBuffer<const FVoxelNodeRangeType>
{
	using TVoxelNodeBuffer::TVoxelNodeBuffer;
};
struct FVoxelNodeOutputRangeBuffer : TVoxelNodeBuffer<FVoxelNodeRangeType>
{
	using TVoxelNodeBuffer::TVoxelNodeBuffer;
};