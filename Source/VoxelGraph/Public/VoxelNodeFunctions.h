// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelMaterial.h"
#include "VoxelRange.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"

namespace FVoxelNodeFunctions
{
	inline void GetDoubleIndex(const FVoxelMaterial& Material, int32& OutA, int32& OutB, float& OutBlend)
	{
		OutA = Material.GetIndexA();
		OutB = Material.GetIndexB();
		OutBlend = Material.GetBlend() / 255.999f;
	}
	inline void GetDoubleIndex(const FVoxelMaterialRange& Material, TVoxelRange<int32>& OutA, TVoxelRange<int32>& OutB, TVoxelRange<float>& OutBlend)
	{
		OutA = { 0, 255 };
		OutB = { 0, 255 };
		OutBlend = { 0, 1 };
	}

	inline void GetColor(const FVoxelMaterial& Material, float& OutR, float& OutG, float& OutB, float& OutA)
	{
		FLinearColor Color = Material.GetLinearColor();
		OutR = Color.R;
		OutG = Color.G;
		OutB = Color.B;
		OutA = Color.A;
	}
	inline void GetColor(const FVoxelMaterialRange& Material, TVoxelRange<float>& OutR, TVoxelRange<float>& OutG, TVoxelRange<float>& OutB, TVoxelRange<float>& OutA)
	{
		OutR = { 0, 1 };
		OutG = { 0, 1 };
		OutB = { 0, 1 };
		OutA = { 0, 1 };
	}
	// For back compatibility
	inline void GetColor(const FVoxelMaterial& Material, float& OutR, float& OutG, float& OutB)
	{
		float Dummy;
		GetColor(Material, OutR, OutG, OutB, Dummy);
	}

	inline float Sqrt(float F)
	{
		if (F <= 0)
		{
			return 0;
		}
		else
		{
			return FMath::Sqrt(F);
		}
	}
	inline TVoxelRange<float> Sqrt(const TVoxelRange<float>& F)
	{
		return { FMath::Sqrt(FMath::Max(0.f, F.Min)), FMath::Sqrt(FMath::Max(0.f, F.Max)) };
	}

	inline float VectorLength(float X, float Y, float Z)
	{
		return FMath::Sqrt(X*X + Y*Y + Z*Z);
	}
	inline TVoxelRange<float> VectorLength(const TVoxelRange<float>& X, const TVoxelRange<float>& Y, const TVoxelRange<float>& Z)
	{
		return Sqrt(X*X + Y*Y + Z*Z);
	}

	inline void VectorRotateAngleAxis(float X, float Y, float Z, float AxisX, float AxisY, float AxisZ, float Angle, float& OutX, float& OutY, float& OutZ)
	{
		// taken from FVector; not using directly to keep from allocating an fvector
		float S, C;
		FMath::SinCos(&S, &C, FMath::DegreesToRadians(Angle));

		const float XX = AxisX * AxisX;
		const float YY = AxisY * AxisY;
		const float ZZ = AxisZ * AxisZ;

		const float XY = AxisX * AxisY;
		const float YZ = AxisY * AxisZ;
		const float ZX = AxisZ * AxisX;

		const float XS = AxisX * S;
		const float YS = AxisY * S;
		const float ZS = AxisZ * S;

		const float OMC = 1.f - C;

		OutX = (OMC * XX + C) * X + (OMC * XY - ZS) * Y + (OMC * ZX + YS) * Z;
		OutY = (OMC * XY + ZS) * X + (OMC * YY + C) * Y + (OMC * YZ - XS) * Z;
		OutZ = (OMC * ZX - YS) * X + (OMC * YZ + XS) * Y + (OMC * ZZ + C) * Z;
	}
	inline void VectorRotateAngleAxis(
		const TVoxelRange<float>& X,
		const TVoxelRange<float>& Y,
		const TVoxelRange<float>& Z, 
		const TVoxelRange<float>& AxisX,
		const TVoxelRange<float>& AxisY,
		const TVoxelRange<float>& AxisZ,
		const TVoxelRange<float>& Angle,
		TVoxelRange<float>& OutX, 
		TVoxelRange<float>& OutY, 
		TVoxelRange<float>& OutZ)
	{
		FVoxelRangeFailStatus::Fail();
	}

	inline float GetPerlinWormsDistance(const FVoxelPlaceableItemHolder& ItemHolder, const FVector& Position)
	{
		float Dist = 0;
		return Dist;
	}
	inline float GetPerlinWormsDistance(const FVoxelPlaceableItemHolder& ItemHolder, int32 X, int32 Y, int32 Z)
	{
		return GetPerlinWormsDistance(ItemHolder, FVector(X, Y, Z));
	}
	inline TVoxelRange<float> GetPerlinWormsDistance(const FVoxelPlaceableItemHolder& ItemHolder, const TVoxelRange<int32>& X, const TVoxelRange<int32>& Y, const TVoxelRange<int32>& Z)
	{
		return { 0, 0 };
	}

	inline int32 RoundToInt(float Value)
	{
		return FMath::RoundToInt(Value);
	}
	inline TVoxelRange<int32> RoundToInt(const TVoxelRange<float>& Value)
	{
		return { FMath::FloorToInt(Value.Min), FMath::CeilToInt(Value.Max) };
	}

	inline float Lerp(float A, float B, float Alpha)
	{
		return FMath::Lerp(A, B, Alpha);
	}
	inline TVoxelRange<float> Lerp(const TVoxelRange<float>& A, const TVoxelRange<float>& B, const TVoxelRange<float>& Alpha)
	{
		return A + Alpha * (B - A);
	}

	inline float Clamp(float Value, float Min, float Max)
	{
		return FMath::Clamp(Value, Min, Max);
	}
	inline TVoxelRange<float> Clamp(const TVoxelRange<float>& Value, const TVoxelRange<float>& Min, const TVoxelRange<float>& Max)
	{
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

	inline float Pow(float A, float B)
	{
		return FMath::Pow(A, B);
	}
	inline TVoxelRange<float> Pow(const TVoxelRange<float>& A, const TVoxelRange<float>& B)
	{
		if (B.IsSingleValue())
		{
			float Exp = B.Min;
			int32 IntExp = FMath::RoundToInt(Exp);
			if (Exp == IntExp) // If integer
			{
				if (IntExp % 2 == 0) // If multiple of 2: decreasing [-infinity, 0] and increasing [0, infinity]
				{
					if (0 <= A.Min)
					{
						return { FMath::Pow(A.Min, Exp), FMath::Pow(A.Max, Exp) };
					}
					else if (A.Max <= 0)
					{
						return { FMath::Pow(A.Max, Exp), FMath::Pow(A.Min, Exp) };
					}
					else
					{
						return { 0, FMath::Max(FMath::Pow(A.Max, Exp), FMath::Pow(A.Min, Exp)) };
					}
				}
				else // Increasing
				{
					return { FMath::Pow(A.Min, Exp), FMath::Pow(A.Max, Exp) };
				}
			}
			else
			{
				// If not integer then pow(x, exp) = 0 if x < 0
				if (0 <= A.Min)
				{
					return { FMath::Pow(A.Min, Exp), FMath::Pow(A.Max, Exp) };
				}
				else if (A.Max <= 0)
				{
					return { 0, 0 };
				}
				else
				{
					return { 0, FMath::Pow(A.Max, Exp) };
				}
			}
		}
		else
		{
			FVoxelRangeFailStatus::Fail();
			return { 0, 0 };
		}
	}

	template<typename T>
	inline T Abs(T A)
	{
		return FMath::Abs(A);
	}
	template<typename T>
	inline TVoxelRange<T> Abs(const TVoxelRange<T>& A)
	{
		return { FMath::Min(FMath::Abs(A.Min), FMath::Abs(A.Max)), FMath::Max(FMath::Abs(A.Min), FMath::Abs(A.Max)) };
	}

	inline int32 CeilToInt(float A)
	{
		return FMath::CeilToInt(A);
	}
	inline TVoxelRange<int32> CeilToInt(const TVoxelRange<float>& A)
	{
		return { FMath::CeilToInt(A.Min), FMath::CeilToInt(A.Max) };
	}

	inline int32 FloorToInt(float A)
	{
		return FMath::FloorToInt(A);
	}
	inline TVoxelRange<int32> FloorToInt(const TVoxelRange<float>& A)
	{
		return { FMath::FloorToInt(A.Min), FMath::FloorToInt(A.Max) };
	}

	inline float Fractional(float A)
	{
		return FMath::Fractional(A);
	}
	inline TVoxelRange<float> Fractional(const TVoxelRange<float>& A)
	{
		if (0 <= A.Min)
		{
			return { 0, 1 };
		}
		else if (A.Max <= 0)
		{
			return { -1, 0 };
		}
		else
		{
			return { -1, 1 };
		}
	}

	template<typename T>
	inline T Sign(T A)
	{
		return FMath::Sign(A);
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

	inline float InvSqrt(float A)
	{
		if (A <= 0)
		{
			return 0;
		}
		else
		{
			return FMath::InvSqrt(A);
		}
	}
	inline TVoxelRange<float> InvSqrt(const TVoxelRange<float>& A)
	{
		if (A.Max <= 0)
		{
			return { 0, 0 };
		}
		else if (0 < A.Min)
		{
			return { FMath::InvSqrt(A.Max), FMath::InvSqrt(A.Min) };
		}
		else
		{
			FVoxelRangeFailStatus::Fail();
			return { 0, 0 };
		}
	}

	inline float Loge(float A)
	{
		if (A <= 0)
		{
			return 0;
		}
		else
		{
			return FMath::Loge(A);
		}
	}
	inline TVoxelRange<float> Loge(const TVoxelRange<float>& A)
	{
		if (A.Max <= 0)
		{
			return { 0, 0 };
		}
		else if (0 < A.Min)
		{
			return { FMath::Loge(A.Min), FMath::Loge(A.Max) };
		}
		else
		{
			FVoxelRangeFailStatus::Fail();
			return { 0, 0 };
		}
	}

	inline float Exp(float A)
	{
		return FMath::Exp(A);
	}
	inline TVoxelRange<float> Exp(const TVoxelRange<float>& A)
	{
		return { FMath::Exp(A.Min), FMath::Exp(A.Max) };
	}

	inline float Sinh(float A)
	{
		return FMath::Sinh(A);
	}
	inline TVoxelRange<float> Sinh(const TVoxelRange<float>& A)
	{
		return { FMath::Sinh(A.Min), FMath::Sinh(A.Max) };
	}

	inline float Sin(float A)
	{
		return FMath::Sin(A);
	}
	inline TVoxelRange<float> Sin(const TVoxelRange<float>& A)
	{
		if (A.IsSingleValue())
		{
			return { FMath::Sin(A.Min), FMath::Sin(A.Max) };
		}
		else
		{
			return { -1, 1 };
		}
	}

	inline float Cos(float A)
	{
		return FMath::Cos(A);
	}
	inline TVoxelRange<float> Cos(const TVoxelRange<float>& A)
	{
		if (A.IsSingleValue())
		{
			return { FMath::Cos(A.Min), FMath::Cos(A.Max) };
		}
		else
		{
			return { -1, 1 };
		}
	}

	inline float Asin(float A)
	{
		return FMath::Asin(A);
	}
	inline TVoxelRange<float> Asin(const TVoxelRange<float>& A)
	{
		if (A.IsSingleValue())
		{
			return { FMath::Asin(A.Min), FMath::Asin(A.Max) };
		}
		else
		{
			return { -2, 2 };
		}
	}

	inline float Acos(float A)
	{
		return FMath::Acos(A);
	}
	inline TVoxelRange<float> Acos(const TVoxelRange<float>& A)
	{
		if (A.IsSingleValue())
		{
			return { FMath::Acos(A.Min), FMath::Acos(A.Max) };
		}
		else
		{
			return { 0, 4 };
		}
	}

	inline float Tan(float A)
	{
		return FMath::Tan(A);
	}
	inline TVoxelRange<float> Tan(const TVoxelRange<float>& A)
	{
		if (A.IsSingleValue())
		{
			return { FMath::Tan(A.Min), FMath::Tan(A.Max) };
		}
		else
		{
			FVoxelRangeFailStatus::Fail();
			return { 0, 0 };
		}
	}

	inline float Atan(float A)
	{
		return FMath::Atan(A);
	}
	inline TVoxelRange<float> Atan(const TVoxelRange<float>& A)
	{
		return { FMath::Atan(A.Min), FMath::Atan(A.Max) };
	}

	inline float Atan2(float Y, float X)
	{
		return FMath::Atan2(Y, X);
	}
	inline TVoxelRange<float> Atan2(const TVoxelRange<float>& Y, const TVoxelRange<float>& X)
	{
		return { -4, 4 };
	}

	template<typename T>
	inline T Min(T A, T B)
	{
		return FMath::Min<T>(A, B);
	}
	template<typename T>
	inline TVoxelRange<T> Min(const TVoxelRange<T>& A, const TVoxelRange<T>& B)
	{
		return { FMath::Min(A.Min, B.Min), FMath::Min(A.Max, B.Max) };
	}

	template<typename T>
	inline T Max(T A, T B)
	{
		return FMath::Max<T>(A, B);
	}
	template<typename T>
	inline TVoxelRange<T> Max(const TVoxelRange<T>& A, const TVoxelRange<T>& B)
	{
		return { FMath::Max(A.Min, B.Min), FMath::Max(A.Max, B.Max) };
	}
}
