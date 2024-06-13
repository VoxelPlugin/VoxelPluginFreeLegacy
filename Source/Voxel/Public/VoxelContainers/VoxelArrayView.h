// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"

template<typename T>
struct TVoxelArrayView
{
	TVoxelArrayView() = default;

	TVoxelArrayView(TArray<T>& Other)
		: DataPtr(Other.GetData())
		, ArrayNum(Other.Num())
	{
	}
	template<typename = typename TEnableIf<TIsConst<T>::Value>::Type>
	TVoxelArrayView(const TArray<typename UE_503_SWITCH(TRemoveConst<T>::Type, std::remove_const_t<T>)>& Other)
		: DataPtr(Other.GetData())
		, ArrayNum(Other.Num())
	{
	}
	
	FORCEINLINE bool IsValidIndex(int32 Index) const
	{
		return (Index >= 0) && (Index < ArrayNum);
	}
	FORCEINLINE int32 Num() const
	{
		return ArrayNum;
	}
	FORCEINLINE const T& operator[](int32 Index) const
	{
		checkVoxelSlow(IsValidIndex(Index));
		return DataPtr[Index];
	}

private:
	T* RESTRICT DataPtr = nullptr;
	int32 ArrayNum = 0;
};