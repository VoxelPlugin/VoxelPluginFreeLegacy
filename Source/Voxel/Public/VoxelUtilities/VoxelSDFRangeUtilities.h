// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRangeUtilities.h"

namespace FVoxelSDFRangeUtilities
{
#if !INTELLISENSE_PARSER
	using flt = TVoxelRange<v_flt>;
	
	namespace Impl
	{
		FORCEINLINE flt Min(flt a, flt b) { return FVoxelRangeUtilities::Min(a, b); }
		FORCEINLINE flt Max(flt a, flt b) { return FVoxelRangeUtilities::Max(a, b); }
		FORCEINLINE flt Clamp(flt x, flt a, flt b) { return FVoxelRangeUtilities::Clamp(x, a, b); }
		FORCEINLINE flt Abs(flt x) { return FVoxelRangeUtilities::Abs(x); }
		FORCEINLINE flt Sign(flt x) { return FVoxelRangeUtilities::Sign(x); }
		FORCEINLINE flt Sqrt(flt x) { return FVoxelRangeUtilities::Sqrt(x); }
		FORCEINLINE flt Lerp(flt a, flt b, flt x) { return FVoxelRangeUtilities::Lerp(a, b, x); }
	}
#endif

#include "VoxelSDFUtilitiesImpl.inl"
}