// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"

namespace FVoxelSDFUtilities
{
#if !INTELLISENSE_PARSER
	using flt = v_flt;
	
	namespace Impl
	{
		FORCEINLINE flt Min(flt a, flt b) { return FMath::Min(a, b); }
		FORCEINLINE flt Max(flt a, flt b) { return FMath::Max(a, b); }
		FORCEINLINE flt Clamp(flt x, flt a, flt b) { return FMath::Clamp(x, a, b); }
		FORCEINLINE flt Abs(flt x) { return FMath::Abs(x); }
		FORCEINLINE flt Sign(flt x) { return FMath::Sign(x); }
		FORCEINLINE flt Sqrt(flt x) { return FMath::Sqrt(x); }
		FORCEINLINE flt Lerp(flt a, flt b, flt x) { return FMath::Lerp(a, b, x); }
	}
#endif

#include "VoxelSDFUtilitiesImpl.inl"
}