// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRange.h"
#include "VoxelIntBox.h"

#include <cmath>

namespace FVoxelRangeUtilities
{
	template<typename T>
	inline TVoxelRange<T> Min(const TVoxelRange<T>& A, const TVoxelRange<T>& B)
	{
		return { FMath::Min(A.Min, B.Min), FMath::Min(A.Max, B.Max) };
	}
	template<typename T>
	inline TVoxelRange<T> Max(const TVoxelRange<T>& A, const TVoxelRange<T>& B)
	{
		return { FMath::Max(A.Min, B.Min), FMath::Max(A.Max, B.Max) };
	}
	template<typename T>
	inline TVoxelRange<T> Clamp(const TVoxelRange<T>& Value, const TVoxelRange<T>& Min, const TVoxelRange<T>& Max)
	{
		if (Min.IsSingleValue() && Max.IsSingleValue())
		{
			return
			{
				FMath::Clamp(Value.Min, Min.GetSingleValue(), Max.GetSingleValue()),
				FMath::Clamp(Value.Max, Min.GetSingleValue(), Max.GetSingleValue())
			};
		}
		if (Min.Max <= Value.Min && Value.Max <= Max.Min) // If already clamped
		{
			return Value;
		}
		else if (Max.Max <= Value.Min)
		{
			return Max.Max;
		}
		else if (Value.Max <= Min.Min)
		{
			return Min.Min;
		}
		else
		{
			return { Min.Min, Max.Max };
		}
	}
	template<typename T>
	inline TVoxelRange<T> Abs(const TVoxelRange<T>& A)
	{
		return { A.Contains(0) ? 0 : FMath::Min(FMath::Abs(A.Min), FMath::Abs(A.Max)), FMath::Max(FMath::Abs(A.Min), FMath::Abs(A.Max)) };
	}
	template<typename T>
	inline TVoxelRange<T> Sign(const TVoxelRange<T>& A)
	{
		if (0 < A.Min)
		{
			return { 1, 1 };
		}
		else if (A.Max < 0)
		{
			return { -1, -1 };
		}
		else if (A.Min == 0 && A.Max == 0)
		{
			return { 0, 0 };
		}
		else
		{
			return { -1, 1 };
		}
	}
	inline v_flt Sqrt(v_flt F)
	{
		if (F <= 0)
		{
			return 0;
		}
		else
		{
			return std::sqrt(F);
		}
	}
	inline TVoxelRange<v_flt> Sqrt(const TVoxelRange<v_flt>& F)
	{
		return { Sqrt(FMath::Max<v_flt>(0., F.Min)), Sqrt(FMath::Max<v_flt>(0., F.Max)) };
	}
	inline TVoxelRange<v_flt> Lerp(const TVoxelRange<v_flt>& A, const TVoxelRange<v_flt>& B, const TVoxelRange<v_flt>& Alpha)
	{
		if (Alpha.IsSingleValue() && 0 <= Alpha.GetSingleValue() && Alpha.GetSingleValue() <= 1)
		{
			return 
			{
				FMath::Lerp(A.Min, B.Min, Alpha.GetSingleValue()),
				FMath::Lerp(A.Max, B.Max, Alpha.GetSingleValue())
			};
		}
		else
		{
			return A + Alpha * (B - A);
		}
	}
	inline FVoxelIntBox BoundsFromRanges(TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, TVoxelRange<v_flt> Z)
	{
		return FVoxelIntBox(
			FIntVector(
				FMath::FloorToInt(X.Min),
				FMath::FloorToInt(Y.Min),
				FMath::FloorToInt(Z.Min)),
			FIntVector(
				FMath::CeilToInt(X.Max) + 1,
				FMath::CeilToInt(Y.Max) + 1,
				FMath::CeilToInt(Z.Max) + 1));
	}
}