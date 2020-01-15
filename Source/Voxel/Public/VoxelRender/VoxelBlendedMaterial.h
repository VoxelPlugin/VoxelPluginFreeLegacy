// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

template<bool bSorted>
struct TVoxelBlendedMaterial
{
	enum EKind
	{
		Single = 0,
		Double = 1,
		Triple = 2,
		Invalid = 3
	};

	TVoxelBlendedMaterial() = default;

	TVoxelBlendedMaterial(uint8 Index)
		: Index0(Index)
		, Kind(Single)
	{
	}

	TVoxelBlendedMaterial(uint8 Index0, uint8 Index1)
		: Index0(Index0)
		, Index1(Index1)
		, Kind(Double)
	{
		check(!bSorted || Index0 < Index1);
	}

	TVoxelBlendedMaterial(uint8 Index0, uint8 Index1, uint8 Index2)
		: Index0(Index0)
		, Index1(Index1)
		, Index2(Index2)
		, Kind(Triple)
	{
		check(!bSorted || Index0 < Index1 && Index1 < Index2);
	}

	template<bool T>
	inline bool operator==(const TVoxelBlendedMaterial<T>& Other) const
	{
		return Kind == Other.Kind && Index0 == Other.Index0 && Index1 == Other.Index1 && Index2 == Other.Index2;
	}

	inline FString ToString() const
	{
		switch (Kind)
		{
		case Single:
			return FString::Printf(TEXT("Single %d"), Index0);
		case Double:
			return FString::Printf(TEXT("Double %d %d"), Index0, Index1);
		case Triple:
			return FString::Printf(TEXT("Triple %d %d %d"), Index0, Index1, Index2);
		case Invalid:
		default:
			return "Invalid";
		}
	}

	inline FString KindToString() const
	{
		switch (Kind)
		{
		case Single:
			return "Single";
		case Double:
			return "Double";
		case Triple:
			return "Triple";
		case Invalid:
		default:
			return "Invalid";
		}
	}

	inline TArray<uint8, TFixedAllocator<3>> GetElements() const
	{
		switch (Kind)
		{
		case Single:
			return { Index0 };
		case Double:
			return { Index0, Index1 };
		case Triple:
			return { Index0, Index1, Index2 };
		case Invalid:
		default:
			return {};
		}
	}

	uint8 Index0 = 255;
	uint8 Index1 = 255;
	uint8 Index2 = 255;
	EKind Kind = Invalid;
};

template<bool T>
inline uint32 GetTypeHash(const TVoxelBlendedMaterial<T>& O)
{
	return GetTypeHash(O.Index0) ^ GetTypeHash(O.Index1) ^ GetTypeHash(O.Index2) ^ GetTypeHash(O.Kind);
}

struct FVoxelBlendedMaterialSorted : TVoxelBlendedMaterial<true> { using TVoxelBlendedMaterial<true>::TVoxelBlendedMaterial; };
struct FVoxelBlendedMaterialUnsorted : TVoxelBlendedMaterial<false> { using TVoxelBlendedMaterial<false>::TVoxelBlendedMaterial; };

inline uint32 GetTypeHash(const FVoxelBlendedMaterialSorted& O)
{
	return GetTypeHash(static_cast<const TVoxelBlendedMaterial<true>&>(O));
}
inline uint32 GetTypeHash(const FVoxelBlendedMaterialUnsorted& O)
{
	return GetTypeHash(static_cast<const TVoxelBlendedMaterial<false>&>(O));
}