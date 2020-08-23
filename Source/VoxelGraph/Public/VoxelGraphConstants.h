// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphGlobals.h"
#include "VoxelUtilities/VoxelBaseUtilities.h"

namespace FVoxelGraphOutputsIndices
{
	enum : uint16
	{
		RangeAnalysisIndex	  = 0, // Not used as an actual output
		ValueIndex            = 1,
		MaterialIndex         = 2,
		UpVectorXIndex         = 3,
		UpVectorYIndex         = 4,
		UpVectorZIndex         = 5,
		DefaultOutputsLastUsed = 5,
		DefaultOutputsMax      = 32,
		OutputsMax			   = 256
	};
}

static_assert(FVoxelGraphOutputsIndices::OutputsMax == MAX_VOXELGRAPH_OUTPUTS, "");

namespace FVoxelGraphPermutation
{
	template<uint32 A>
	inline bool constexpr Contains(uint32 Value) { return Value == A; }
	template<uint32 A, uint32 B, uint32... Permutation>
	inline bool constexpr Contains(uint32 Value)
	{
		return Value == A || Contains<B, Permutation...>(Value);
	}
	
	template<uint32 A>
	static inline constexpr bool IsSorted() { return true; }
	static inline constexpr bool IsSorted() { return true; }
	template<uint32 A, uint32 B, uint32... Permutation>
	static inline constexpr bool IsSorted()
	{
		return A <= B && IsSorted<B, Permutation...>();
	}
	
	template<uint32 A>
	inline uint32 constexpr Hash()
	{
		return FVoxelUtilities::MurmurHash32(A);
	}
	template<uint32 A, uint32 B, uint32... Permutation>
	inline uint32 constexpr Hash()
	{
		return FVoxelUtilities::MurmurHash32(FVoxelUtilities::MurmurHash32(A) ^ Hash<B, Permutation...>());
	}

	template<typename TAllocator>
	inline uint32 Hash(const TArray<uint32, TAllocator>& Array, int32 Index = 0)
	{
		if (Array.Num() - Index == 1)
		{
			return FVoxelUtilities::MurmurHash32(Array[Index]);
		}
		else
		{
			return FVoxelUtilities::MurmurHash32(FVoxelUtilities::MurmurHash32(Array[Index]) ^ Hash(Array, Index + 1));
		}
	}
}