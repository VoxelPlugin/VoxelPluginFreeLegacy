// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelUtilities/VoxelBaseUtilities.h"

template<typename T, uint32 Size, uint32 Alignment = alignof(T)>
class alignas(Alignment) TVoxelStaticArray
{
public:
	using ElementType = T;
	
	TVoxelStaticArray()
	{
	}
	FORCEINLINE explicit TVoxelStaticArray(EForceInit)
	{
		for (auto& Element : *this)
		{
			new (&Element) T{};
		}
	}
	template<typename... TArgs>
	FORCEINLINE TVoxelStaticArray(TArgs... Args)
	{
		static_assert(sizeof...(Args) == Size, "");
		SetFromVariadicArgs(Args...);
	}

	FORCEINLINE static constexpr uint32 Num()
	{
		return Size;
	}
	FORCEINLINE static constexpr uint32 GetTypeSize()
	{
		return sizeof(T);
	}

	FORCEINLINE void Memzero()
	{
		FMemory::Memzero(GetData(), Size * sizeof(T));
	}

	FORCEINLINE T* RESTRICT GetData()
	{
		return reinterpret_cast<T*>(Data);
	}
	FORCEINLINE const T* RESTRICT GetData() const
	{
		return reinterpret_cast<const T*>(Data);
	}

	template<typename TFrom, typename Allocator>
	void CopyFromArray(const TArray<TFrom, Allocator>& Array, bool bInitializeEnd = true)
	{
		check(Size >= Array.Num());
		for (int32 Index = 0; Index < Array.Num(); Index++)
		{
			(*this)[Index] = Array[Index];
		}
		if (bInitializeEnd)
		{
			for (int32 Index = Array.Num(); Index < Size; Index++)
			{
				(*this)[Index] = T{};
			}
		}
	}

	FORCEINLINE T& operator[](int32 Index)
	{
		checkVoxelSlow(0 <= Index && Index < Size);
		return GetData()[Index];
	}
	FORCEINLINE const T& operator[](int32 Index) const
	{
		checkVoxelSlow(0 <= Index && Index < Size);
		return GetData()[Index];
	}

	operator TArray<T>() const
	{
		return TArray<T>(GetData(), Num());
	}
	
	operator TArrayView<T>()
	{
		return TArrayView<T>(GetData(), Num());
	}
	operator TArrayView<const T>() const
	{
		return TArrayView<const T>(GetData(), Num());
	}

	FORCEINLINE T* begin() { return GetData(); }
	FORCEINLINE T* end()   { return GetData() + Size; }
	
	FORCEINLINE const T* begin() const { return GetData(); }
	FORCEINLINE const T* end()   const { return GetData() + Size; }

	template<int32 Index = 0, typename... TArgs>
	FORCEINLINE void SetFromVariadicArgs(T Arg, TArgs... Args)
	{
		static_assert(0 <= Index && Index < Size, "");
		static_assert(sizeof...(Args) == Size - 1 - Index, "");
		(*this)[Index] = Arg;
		SetFromVariadicArgs<Index + 1>(Args...);
	}
	template<int32 Index>
	FORCEINLINE void SetFromVariadicArgs(T Arg)
	{
		static_assert(Index == Size - 1, "");
		(*this)[Index] = Arg;
	}

private:
	uint8 Data[Size * sizeof(T)];
};

template<typename T, uint32 Size, uint32 Alignment>
struct TIsContiguousContainer<TVoxelStaticArray<T, Size, Alignment>>
{
	enum { Value = true };
};

template<uint32 Size>
class TVoxelStaticBitArray
{
public:
	TVoxelStaticBitArray() = default;
	TVoxelStaticBitArray(EForceInit)
	{
		Clear();
	}

	void Clear()
	{
		Array.Memzero();
	}

	FORCEINLINE void Set(uint32 Index)
	{
		checkVoxelSlow(Index < Size);
		Array[Index / 32] |= (1u << (Index % 32));
	}
	FORCEINLINE void Clear(uint32 Index)
	{
		checkVoxelSlow(Index < Size);
		Array[Index / 32] &= ~(1u << (Index % 32));
	}
	FORCEINLINE bool Test(uint32 Index) const
	{
		checkVoxelSlow(Index < Size);
		return Array[Index / 32] & (1u << (Index % 32));
	}

private:
	TVoxelStaticArray<uint32, FVoxelUtilities::DivideCeil(Size, 32)> Array;
};