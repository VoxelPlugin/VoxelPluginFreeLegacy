// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"

// TNoGrowArray will ensure if a grow allocation is made
// This is useful to detect incorrect Reserve

template<typename T>
class TNoGrowAllocator : public T
{
public:
	class ForAnyElementType : public T::ForAnyElementType
	{
	public:
		FORCEINLINE typename T::SizeType CalculateSlackGrow(typename T::SizeType NumElements, typename T::SizeType NumAllocatedElements, SIZE_T NumBytesPerElement) const
		{
			ensure(false);
			return T::ForAnyElementType::CalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement);
		}
	};

	template<typename ElementType>
	class ForElementType 
		UE_5_SWITCH( : public T::template ForAnyElementType<ElementType>, : public T::template ForAnyElementType)
	{
	public:
		FORCEINLINE typename T::SizeType CalculateSlackGrow(typename T::SizeType NumElements, typename T::SizeType NumAllocatedElements, SIZE_T NumBytesPerElement) const
		{
			ensure(false);
			return UE_5_SWITCH(T::template ForAnyElementType<ElementType>, T::template ForAnyElementType)::CalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement);
		}
	};
};

template<typename T, typename TAllocator = FDefaultAllocator>
using TNoGrowArray = TArray<T, TNoGrowAllocator<TAllocator>>;

template<typename T, typename TAllocator = FDefaultAllocator64>
using TNoGrowArray64 = TArray<T, TNoGrowAllocator<TAllocator>>;

template<typename T, typename TAllocator>
TArray<T, TAllocator>& FromNoGrowArray(TNoGrowArray<T, TAllocator>& Array)
{
	return reinterpret_cast<TArray<T, TAllocator>&>(Array);
}
template<typename T, typename TAllocator>
const TArray<T, TAllocator>& FromNoGrowArray(const TNoGrowArray<T, TAllocator>& Array)
{
	return reinterpret_cast<const TArray<T, TAllocator>&>(Array);
}

template<typename T, typename TAllocator>
TNoGrowArray<T, TAllocator>& ToNoGrowArray(TArray<T, TAllocator>& Array)
{
	return reinterpret_cast<TNoGrowArray<T, TAllocator>&>(Array);
}
template<typename T, typename TAllocator>
const TNoGrowArray<T, TAllocator>& ToNoGrowArray(const TArray<T, TAllocator>& Array)
{
	return reinterpret_cast<const TNoGrowArray<T, TAllocator>&>(Array);
}