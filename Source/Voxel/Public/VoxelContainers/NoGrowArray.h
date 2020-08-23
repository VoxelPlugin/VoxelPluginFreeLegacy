// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

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
	class ForElementType : public T::template ForAnyElementType<ElementType>
	{
	public:
		FORCEINLINE typename T::SizeType CalculateSlackGrow(typename T::SizeType NumElements, typename T::SizeType NumAllocatedElements, SIZE_T NumBytesPerElement) const
		{
			ensure(false);
			return T::template ForAnyElementType<ElementType>::CalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement);
		}
	};
};

template<typename T, typename TAllocator = FDefaultAllocator>
using TNoGrowArray = TArray<T, TNoGrowAllocator<TAllocator>>;

template<typename T, typename TAllocator = FDefaultAllocator64>
using TNoGrowArray64 = TArray<T, TNoGrowAllocator<TAllocator>>;