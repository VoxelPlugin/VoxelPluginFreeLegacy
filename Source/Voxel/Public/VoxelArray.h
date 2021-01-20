// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"

template<typename T>
class TVoxelArrayFwd;

template<>
class TVoxelArrayFwd<FVoxelValue> : public FVoxelValueArray
{
public:
	using FVoxelValueArray::FVoxelValueArray;
};

template<>
class TVoxelArrayFwd<FVoxelMaterial> : public TArray<FVoxelMaterial>
{
public:
	using TArray<FVoxelMaterial>::TArray;
};

FORCEINLINE decltype(auto) GetData(TVoxelArrayFwd<FVoxelValue>& Array)
{
	return GetData(static_cast<FVoxelValueArray&>(Array));
}
FORCEINLINE decltype(auto) GetData(const TVoxelArrayFwd<FVoxelValue>& Array)
{
	return GetData(static_cast<const FVoxelValueArray&>(Array));
}
FORCEINLINE decltype(auto) GetNum(const TVoxelArrayFwd<FVoxelValue>& Array)
{
	return GetNum(static_cast<const FVoxelValueArray&>(Array));
}

FORCEINLINE decltype(auto) GetData(TVoxelArrayFwd<FVoxelMaterial>& Array)
{
	return GetData(static_cast<TArray<FVoxelMaterial>&>(Array));
}
FORCEINLINE decltype(auto) GetData(const TVoxelArrayFwd<FVoxelMaterial>& Array)
{
	return GetData(static_cast<const TArray<FVoxelMaterial>&>(Array));
}
FORCEINLINE decltype(auto) GetNum(const TVoxelArrayFwd<FVoxelMaterial>& Array)
{
	return GetNum(static_cast<const TArray<FVoxelMaterial>&>(Array));
}

template<typename T, uint32 Size>
class TVoxelStaticArrayFwd;

template<uint32 Size>
class TVoxelStaticArrayFwd<FVoxelValue, Size> : public TVoxelValueStaticArray<Size>
{
public:
	using TVoxelValueStaticArray<Size>::TVoxelValueStaticArray;
};

template<uint32 Size>
class TVoxelStaticArrayFwd<FVoxelMaterial, Size> : public TVoxelStaticArray<FVoxelMaterial, Size>
{
public:
	using TVoxelStaticArray<FVoxelMaterial, Size>::TVoxelStaticArray;
};

template<uint32 Size>
FORCEINLINE decltype(auto) GetData(TVoxelStaticArrayFwd<FVoxelValue, Size>& Array)
{
	return GetData(static_cast<TVoxelStaticArray<FVoxelValue, Size>&>(Array));
}
template<uint32 Size>
FORCEINLINE decltype(auto) GetData(const TVoxelStaticArrayFwd<FVoxelValue, Size>& Array)
{
	return GetData(static_cast<const TVoxelStaticArray<FVoxelValue, Size>&>(Array));
}
template<uint32 Size>
FORCEINLINE decltype(auto) GetNum(const TVoxelStaticArrayFwd<FVoxelValue, Size>& Array)
{
	return GetNum(static_cast<const TVoxelStaticArray<FVoxelValue, Size>&>(Array));
}

template<uint32 Size>
FORCEINLINE decltype(auto) GetData(TVoxelStaticArrayFwd<FVoxelMaterial, Size>& Array)
{
	return GetData(static_cast<TVoxelStaticArray<FVoxelMaterial, Size>&>(Array));
}
template<uint32 Size>
FORCEINLINE decltype(auto) GetData(const TVoxelStaticArrayFwd<FVoxelMaterial, Size>& Array)
{
	return GetData(static_cast<const TVoxelStaticArray<FVoxelMaterial, Size>&>(Array));
}
template<uint32 Size>
FORCEINLINE decltype(auto) GetNum(const TVoxelStaticArrayFwd<FVoxelMaterial, Size>& Array)
{
	return GetNum(static_cast<const TVoxelStaticArray<FVoxelMaterial, Size>&>(Array));
}