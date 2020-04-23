// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelBaseUtilities.h"

template<typename T, uint32 Size>
class TStackArray
{
public:
	using ElementType = T;
	
	TStackArray()
	{
	}
	explicit TStackArray(EForceInit)
	{
		for (auto& Element : *this)
		{
			new (&Element) T{};
		}
	}

	FORCEINLINE static constexpr uint32 Num()
	{
		return Size;
	}
	FORCEINLINE static constexpr uint32 GetTypeSize()
	{
		return sizeof(T);
	}

	inline void Memzero()
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

	inline operator TArray<T>() const
	{
		return TArray<T>(GetData(), Num());
	}

	FORCEINLINE T* begin() { return GetData(); }
	FORCEINLINE T* end()   { return GetData() + Size; }
	
	FORCEINLINE const T* begin() const { return GetData(); }
	FORCEINLINE const T* end()   const { return GetData() + Size; }

private:
	uint8 Data[Size * sizeof(T)];
};

template<uint32 Size>
class TStackBitArray
{
public:
	TStackBitArray() = default;
	TStackBitArray(EForceInit)
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
	FORCEINLINE bool Test(uint32 Index)
	{
		checkVoxelSlow(Index < Size);
		return Array[Index / 32] & (1u << (Index % 32));
	}

private:
	TStackArray<uint32, FVoxelUtilities::DivideCeil(Size, 32)> Array;
};