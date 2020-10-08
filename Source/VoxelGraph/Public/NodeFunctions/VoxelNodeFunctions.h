// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelContext.h"
#include "VoxelRange.h"
#include "VoxelTexture.h"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelUtilities/VoxelRangeUtilities.h"
#include "VoxelUtilities/VoxelIntVectorUtilities.h"
#include "VoxelUtilities/VoxelRichCurveUtilities.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"
#include "Curves/RichCurve.h"
#include "VoxelGraphGlobals.h"

class UTexture2D;
class UCurveFloat;
class UCurveLinearColor;

struct VOXELGRAPH_API FVoxelRichCurve
{
	FRichCurve Curve;
	
	inline float GetMin() const { return Min; }
	inline float GetMax() const { return Max; }

	FVoxelRichCurve() = default;
	explicit FVoxelRichCurve(const FRichCurve& Curve);
	explicit FVoxelRichCurve(const UCurveFloat* Curve);

private:
	float Min = 0;
	float Max = 0;
};

struct VOXELGRAPH_API FVoxelColorRichCurve
{
	FVoxelRichCurve Curves[4];

	FVoxelColorRichCurve() = default;
	FVoxelColorRichCurve(const UCurveLinearColor* Curve);
};

namespace FVoxelNodeFunctions
{
	inline v_flt Sqrt(v_flt F)
	{
		return FVoxelRangeUtilities::Sqrt(F);
	}
	inline TVoxelRange<v_flt> Sqrt(const TVoxelRange<v_flt>& F)
	{
		return FVoxelRangeUtilities::Sqrt(F);
	}

	inline v_flt VectorLength(v_flt X, v_flt Y, v_flt Z)
	{
		return Sqrt(X * X + Y * Y + Z * Z);
	}
	inline TVoxelRange<v_flt> VectorLength(const TVoxelRange<v_flt>& X, const TVoxelRange<v_flt>& Y, const TVoxelRange<v_flt>& Z)
	{
		return Sqrt(X * X + Y * Y + Z * Z);
	}

	inline void VectorRotateAngleAxis(v_flt X, v_flt Y, v_flt Z, v_flt AxisX, v_flt AxisY, v_flt AxisZ, v_flt Angle, v_flt& OutX, v_flt& OutY, v_flt& OutZ)
	{
		// taken from FVector; not using directly to keep from allocating a FVector
		float S, C;
		FMath::SinCos(&S, &C, FMath::DegreesToRadians(Angle));

		const v_flt XX = AxisX * AxisX;
		const v_flt YY = AxisY * AxisY;
		const v_flt ZZ = AxisZ * AxisZ;

		const v_flt XY = AxisX * AxisY;
		const v_flt YZ = AxisY * AxisZ;
		const v_flt ZX = AxisZ * AxisX;

		const v_flt XS = AxisX * S;
		const v_flt YS = AxisY * S;
		const v_flt ZS = AxisZ * S;

		const v_flt OMC = 1. - C;

		OutX = (OMC * XX + C) * X + (OMC * XY - ZS) * Y + (OMC * ZX + YS) * Z;
		OutY = (OMC * XY + ZS) * X + (OMC * YY + C) * Y + (OMC * YZ - XS) * Z;
		OutZ = (OMC * ZX - YS) * X + (OMC * YZ + XS) * Y + (OMC * ZZ + C) * Z;
	}
	inline void VectorRotateAngleAxis(
		const TVoxelRange<v_flt>& X,
		const TVoxelRange<v_flt>& Y,
		const TVoxelRange<v_flt>& Z,
		const TVoxelRange<v_flt>& AxisX,
		const TVoxelRange<v_flt>& AxisY,
		const TVoxelRange<v_flt>& AxisZ,
		const TVoxelRange<v_flt>& Angle,
		TVoxelRange<v_flt>& OutX,
		TVoxelRange<v_flt>& OutY,
		TVoxelRange<v_flt>& OutZ)
	{
		if (X.IsSingleValue() && 
			Y.IsSingleValue() && 
			Z.IsSingleValue() && 
			AxisX.IsSingleValue() && 
			AxisY.IsSingleValue() && 
			AxisZ.IsSingleValue() &&
			Angle.IsSingleValue())
		{
			v_flt OutXF, OutYF, OutZF;
			VectorRotateAngleAxis(
				X.GetSingleValue(),
				Y.GetSingleValue(),
				Z.GetSingleValue(),
				AxisX.GetSingleValue(),
				AxisY.GetSingleValue(),
				AxisZ.GetSingleValue(),
				Angle.GetSingleValue(),
				OutXF,
				OutYF,
				OutZF);

			OutX = OutXF;
			OutY = OutYF;
			OutZ = OutZF;
		}
		else
		{
			FVoxelRangeFailStatus::Get().Warning(TEXT("VectorRotateAngleAxis doesn't support range analysis"));
			OutX = TVoxelRange<v_flt>::Infinite();
			OutY = TVoxelRange<v_flt>::Infinite();
			OutZ = TVoxelRange<v_flt>::Infinite();
		}
	}

	inline int32 RoundToInt(v_flt Value)
	{
		return FMath::RoundToInt(Value);
	}
	inline TVoxelRange<int32> RoundToInt(const TVoxelRange<v_flt>& Value)
	{
		return { FMath::FloorToInt(Value.Min), FMath::CeilToInt(Value.Max) };
	}

	inline v_flt Lerp(v_flt A, v_flt B, v_flt Alpha)
	{
		return FMath::Lerp(A, B, Alpha);
	}
	inline TVoxelRange<v_flt> Lerp(const TVoxelRange<v_flt>& A, const TVoxelRange<v_flt>& B, const TVoxelRange<v_flt>& Alpha)
	{
		return FVoxelRangeUtilities::Lerp(A, B, Alpha);
	}

	inline v_flt SafeLerp(v_flt A, v_flt B, v_flt Alpha)
	{
		return FMath::Lerp(A, B, FMath::Clamp<v_flt>(Alpha, 0, 1));
	}
	inline TVoxelRange<v_flt> SafeLerp(const TVoxelRange<v_flt>& A, const TVoxelRange<v_flt>& B, const TVoxelRange<v_flt>& Alpha)
	{
		if (Alpha.IsSingleValue())
		{
			return { SafeLerp(A.Min, B.Min, Alpha.GetSingleValue()), SafeLerp(A.Max, B.Max, Alpha.GetSingleValue()) };
		}
		return
		{
			FMath::Min(SafeLerp(A.Min, B.Min, Alpha.Min), SafeLerp(A.Min, B.Min, Alpha.Max)),
			FMath::Max(SafeLerp(A.Max, B.Max, Alpha.Min), SafeLerp(A.Max, B.Max, Alpha.Max))
		};
	}

	template<typename T>
	inline T Clamp(T Value, T Min, T Max)
	{
		return FMath::Clamp(Value, Min, Max);
	}
	template<typename T>
	inline TVoxelRange<T> Clamp(const TVoxelRange<T>& Value, const TVoxelRange<T>& Min, const TVoxelRange<T>& Max)
	{
		return FVoxelRangeUtilities::Clamp(Value, Min, Max);
	}

	inline int32 RightShift(int32 A, int32 B)
	{
		return A >> FMath::Clamp(B, 0, 32);
	}
	inline TVoxelRange<int32> RightShift(TVoxelRange<int32> A, TVoxelRange<int32> B)
	{
		// NOTE: Does not take into account overflows
		return TVoxelRange<int32>::FromList(
			RightShift(A.Min, B.Min),
			RightShift(A.Min, B.Max),
			RightShift(A.Max, B.Min),
			RightShift(A.Max, B.Max));
	}

	inline int32 LeftShift(int32 A, int32 B)
	{
		return A << FMath::Clamp(B, 0, 32);
	}
	inline TVoxelRange<int32> LeftShift(TVoxelRange<int32> A, TVoxelRange<int32> B)
	{
		// NOTE: Does not take into account overflows
		return TVoxelRange<int32>::FromList(
			RightShift(A.Min, B.Min),
			RightShift(A.Min, B.Max),
			RightShift(A.Max, B.Min),
			RightShift(A.Max, B.Max));
	}

	inline v_flt Pow(v_flt A, v_flt B)
	{
		return std::pow(A, B);
	}
	inline TVoxelRange<v_flt> Pow(const TVoxelRange<v_flt>& A, const TVoxelRange<v_flt>& B)
	{
		if (B.IsSingleValue())
		{
			const v_flt Exp = B.GetSingleValue();
			const int32 IntExp = FMath::RoundToInt(Exp);
			if (Exp == IntExp) // If integer
			{
				if (IntExp % 2 == 0) // If multiple of 2: decreasing [-infinity, 0] and increasing [0, infinity]
				{
					if (0 <= A.Min)
					{
						return { Pow(A.Min, Exp), Pow(A.Max, Exp) };
					}
					else if (A.Max <= 0)
					{
						return { Pow(A.Max, Exp), Pow(A.Min, Exp) };
					}
					else
					{
						return { 0, FMath::Max(Pow(A.Max, Exp), Pow(A.Min, Exp)) };
					}
				}
				else // Increasing
				{
					return { Pow(A.Min, Exp), Pow(A.Max, Exp) };
				}
			}
			else
			{
				// If not integer then pow(x, exp) = 0 if x < 0
				if (0 <= A.Min)
				{
					return { Pow(A.Min, Exp), Pow(A.Max, Exp) };
				}
				else if (A.Max <= 0)
				{
					return { 0, 0 };
				}
				else
				{
					return { 0, Pow(A.Max, Exp) };
				}
			}
		}
		else
		{
			FVoxelRangeFailStatus::Get().Warning(TEXT("pow only supports range analysis with a constant exponent"));
			return TVoxelRange<v_flt>::Infinite();
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
		return FVoxelRangeUtilities::Abs(A);
	}

	inline int32 CeilToInt(v_flt A)
	{
		return FMath::CeilToInt(A);
	}
	inline TVoxelRange<int32> CeilToInt(const TVoxelRange<v_flt>& A)
	{
		return { FMath::CeilToInt(A.Min), FMath::CeilToInt(A.Max) };
	}

	inline int32 FloorToInt(v_flt A)
	{
		return FMath::FloorToInt(A);
	}
	inline TVoxelRange<int32> FloorToInt(const TVoxelRange<v_flt>& A)
	{
		return { FMath::FloorToInt(A.Min), FMath::FloorToInt(A.Max) };
	}

	inline v_flt Fractional(v_flt A)
	{
		if (TIsSame<v_flt, float>::Value)
		{
			return FMath::Fractional(A);
		}
		else
		{
			return A - int64(A);
		}
	}
	inline TVoxelRange<v_flt> Fractional(const TVoxelRange<v_flt>& A)
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
		return FVoxelRangeUtilities::Sign(A);
	}

	inline v_flt InvSqrt(v_flt A)
	{
		if (A <= 0)
		{
			return 0;
		}
		else
		{
			if (TIsSame<v_flt, float>::Value)
			{
				return FMath::InvSqrt(A);
			}
			else
			{
				return 1. / std::sqrt(A);
			}
		}
	}

	inline TVoxelRange<v_flt> InvSqrt(const TVoxelRange<v_flt>& A)
	{
		if (A.IsSingleValue())
		{
			return InvSqrt(A.GetSingleValue());
		}

		if (A.Max <= 0)
		{
			return { 0, 0 };
		}
		else if (0 < A.Min)
		{
			return { InvSqrt(A.Max), InvSqrt(A.Min) };
		}
		else
		{
			FVoxelRangeFailStatus::Get().Warning(TEXT("inv sqrt does not support range analysis with ranges containing 0"));
			return TVoxelRange<v_flt>::Infinite();
		}
	}

	inline v_flt Loge(v_flt A)
	{
		if (A <= 0)
		{
			return 0;
		}
		else
		{
			return std::log(A);
		}
	}
	inline TVoxelRange<v_flt> Loge(const TVoxelRange<v_flt>& A)
	{
		if (A.IsSingleValue())
		{
			return Loge(A.GetSingleValue());
		}

		if (A.Max <= 0)
		{
			return { 0, 0 };
		}
		else if (0 < A.Min)
		{
			return { Loge(A.Min), Loge(A.Max) };
		}
		else
		{
			FVoxelRangeFailStatus::Get().Warning(TEXT("loge does not support range analysis with ranges containing 0"));
			return TVoxelRange<v_flt>::Infinite();
		}
	}

	inline v_flt Exp(v_flt A)
	{
		return std::exp(A);
	}
	inline TVoxelRange<v_flt> Exp(const TVoxelRange<v_flt>& A)
	{
		return { Exp(A.Min), Exp(A.Max) };
	}

	inline v_flt Sinh(v_flt A)
	{
		return std::sinh(A);
	}
	inline TVoxelRange<v_flt> Sinh(const TVoxelRange<v_flt>& A)
	{
		return { Sinh(A.Min), Sinh(A.Max) };
	}

	inline v_flt Sin(v_flt A)
	{
		return std::sin(A);
	}
	inline TVoxelRange<v_flt> Sin(const TVoxelRange<v_flt>& A)
	{
		if (A.IsSingleValue())
		{
			return Sin(A.GetSingleValue());
		}
		else
		{
			return { -1, 1 };
		}
	}

	inline v_flt Cos(v_flt A)
	{
		return std::cos(A);
	}
	inline TVoxelRange<v_flt> Cos(const TVoxelRange<v_flt>& A)
	{
		if (A.IsSingleValue())
		{
			return Cos(A.GetSingleValue());
		}
		else
		{
			return { -1, 1 };
		}
	}

	inline v_flt Asin(v_flt A)
	{
		return std::asin(A);
	}
	inline TVoxelRange<v_flt> Asin(const TVoxelRange<v_flt>& A)
	{
		if (A.IsSingleValue())
		{
			return Asin(A.GetSingleValue());
		}
		else
		{
			return { -2, 2 };
		}
	}

	inline v_flt Acos(v_flt A)
	{
		return std::acos(A);
	}
	inline TVoxelRange<v_flt> Acos(const TVoxelRange<v_flt>& A)
	{
		if (A.IsSingleValue())
		{
			return Acos(A.GetSingleValue());
		}
		else
		{
			return { 0, 4 };
		}
	}

	inline void SinCos(v_flt A, v_flt& OutSin, v_flt& OutCos)
	{
#if VOXEL_DOUBLE_PRECISION
		OutSin = std::sin(A);
		OutCos = std::cos(A);
#else
		FMath::SinCos(&OutSin, &OutCos, A);
#endif
	}
	inline void SinCos(const TVoxelRange<v_flt>& A, TVoxelRange<v_flt>& OutSin, TVoxelRange<v_flt>& OutCos)
	{
		OutSin = Sin(A);
		OutCos = Cos(A);
	}

	inline v_flt Tan(v_flt A)
	{
		return std::tan(A);
	}
	inline TVoxelRange<v_flt> Tan(const TVoxelRange<v_flt>& A)
	{
		if (A.IsSingleValue())
		{
			return Tan(A.GetSingleValue());
		}
		else if (-PI / 2 < A.Min && A.Max < PI / 2)
		{
			return { Tan(A.Min), Tan(A.Max) };
		}
		else
		{
			FVoxelRangeFailStatus::Get().Warning(TEXT("tan does not support range analysis for values outside [-pi/2, pi/2]"));
			return TVoxelRange<v_flt>::Infinite();
		}
	}

	inline v_flt Atan(v_flt A)
	{
		return std::atan(A);
	}
	inline TVoxelRange<v_flt> Atan(const TVoxelRange<v_flt>& A)
	{
		return { Atan(A.Min), Atan(A.Max) };
	}

	inline v_flt Atan2(v_flt Y, v_flt X)
	{
		return std::atan2(Y, X);
	}
	inline TVoxelRange<v_flt> Atan2(const TVoxelRange<v_flt>& Y, const TVoxelRange<v_flt>& X)
	{
		return { -PI / 2, PI / 2 };
	}

	template<typename T>
	inline T Min(T A, T B)
	{
		return FMath::Min<T>(A, B);
	}
	template<typename T>
	inline TVoxelRange<T> Min(const TVoxelRange<T>& A, const TVoxelRange<T>& B)
	{
		return FVoxelRangeUtilities::Min(A, B);
	}

	template<typename T>
	inline T Max(T A, T B)
	{
		return FMath::Max<T>(A, B);
	}
	template<typename T>
	inline TVoxelRange<T> Max(const TVoxelRange<T>& A, const TVoxelRange<T>& B)
	{
		return FVoxelRangeUtilities::Max(A, B);
	}

	inline v_flt Union(v_flt A, v_flt B)
	{
		return 0;
	}
	inline TVoxelRange<v_flt> Union(const TVoxelRange<v_flt>& A, const TVoxelRange<v_flt>& B)
	{
		return TVoxelRange<v_flt>::Union(A, B);
	}

	inline bool IsSingleBool(bool bValue)
	{
		return true;
	}
	inline FVoxelBoolRange IsSingleBool(FVoxelBoolRange Bool)
	{
		return !(Bool.bCanBeTrue && Bool.bCanBeFalse);
	}

	inline v_flt GetCurveValue(const FVoxelRichCurve& Curve, v_flt Value)
	{
		return FVoxelRichCurveUtilities::Eval(Curve.Curve, Value);
	}
	VOXELGRAPH_API TVoxelRange<v_flt> GetCurveValue(const FVoxelRichCurve& Curve, const TVoxelRange<v_flt>& Value);
	
	inline void ReadColorTextureDataFloat(
		const TVoxelTexture<FColor>& Texture,
		const EVoxelSamplerMode Mode,
		v_flt U,
		v_flt V,
		v_flt& OutR,
		v_flt& OutG,
		v_flt& OutB,
		v_flt& OutA)
	{
		const FLinearColor Color = Texture.Sample<FLinearColor>(U, V, Mode);
		OutR = Color.R;
		OutG = Color.G;
		OutB = Color.B;
		OutA = Color.A;
	}
	inline void ReadColorTextureDataFloat(
		const TVoxelTexture<FColor>& Texture,
		const EVoxelSamplerMode Mode,
		const TVoxelRange<v_flt>& U,
		const TVoxelRange<v_flt>& V,
		TVoxelRange<v_flt>& OutR,
		TVoxelRange<v_flt>& OutG,
		TVoxelRange<v_flt>& OutB,
		TVoxelRange<v_flt>& OutA)
	{
		if (U.IsSingleValue() && V.IsSingleValue())
		{
			v_flt R, G, B, A;
			ReadColorTextureDataFloat(Texture, Mode, U.GetSingleValue(), V.GetSingleValue(), R, G, B, A);
			OutR = R;
			OutG = G;
			OutB = B;
			OutA = A;
		}
		else
		{
			OutR = { 0, 1 };
			OutG = { 0, 1 };
			OutB = { 0, 1 };
			OutA = { 0, 1 };
		}
	}
	inline void ReadColorTextureDataInt(
		const TVoxelTexture<FColor>& Texture,
		const EVoxelSamplerMode Mode,
		int32 U,
		int32 V,
		v_flt& OutR,
		v_flt& OutG,
		v_flt& OutB,
		v_flt& OutA)
	{
		const FLinearColor Color = Texture.Sample<FLinearColor>(U, V, Mode);
		OutR = Color.R;
		OutG = Color.G;
		OutB = Color.B;
		OutA = Color.A;
	}
	inline void ReadColorTextureDataInt(
		const TVoxelTexture<FColor>& Texture,
		const EVoxelSamplerMode Mode,
		const TVoxelRange<int32>& U,
		const TVoxelRange<int32>& V,
		TVoxelRange<v_flt>& OutR,
		TVoxelRange<v_flt>& OutG,
		TVoxelRange<v_flt>& OutB,
		TVoxelRange<v_flt>& OutA)
	{
		if (U.IsSingleValue() && V.IsSingleValue())
		{
			v_flt R, G, B, A;
			ReadColorTextureDataInt(Texture, Mode, U.GetSingleValue(), V.GetSingleValue(), R, G, B, A);
			OutR = R;
			OutG = G;
			OutB = B;
			OutA = A;
		}
		else
		{
			OutR = { 0, 1 };
			OutG = { 0, 1 };
			OutB = { 0, 1 };
			OutA = { 0, 1 };
		}
	}
	
	inline v_flt ReadFloatTextureDataFloat(
		const TVoxelTexture<float>& Texture,
		const EVoxelSamplerMode Mode,
		v_flt U,
		v_flt V)
	{
		return Texture.Sample<float>(U, V, Mode);
	}
	inline TVoxelRange<v_flt> ReadFloatTextureDataFloat(
		const TVoxelTexture<float>& Texture,
		const EVoxelSamplerMode Mode,
		const TVoxelRange<v_flt>& U,
		const TVoxelRange<v_flt>& V)
	{
		if (U.IsSingleValue() && V.IsSingleValue())
		{
			return ReadFloatTextureDataFloat(Texture, Mode, U.GetSingleValue(), V.GetSingleValue());
		}
		else
		{
			ensure(Texture.GetMin() <= Texture.GetMax());
			return { Texture.GetMin(), Texture.GetMax() };
		}
	}
	inline v_flt ReadFloatTextureDataInt(
		const TVoxelTexture<float>& Texture,
		const EVoxelSamplerMode Mode,
		int32 U,
		int32 V)
	{
		return Texture.Sample<float>(U, V, Mode);
	}
	inline TVoxelRange<v_flt> ReadFloatTextureDataInt(
		const TVoxelTexture<float>& Texture,
		const EVoxelSamplerMode Mode,
		const TVoxelRange<int32>& U,
		const TVoxelRange<int32>& V)
	{
		if (U.IsSingleValue() && V.IsSingleValue())
		{
			return ReadFloatTextureDataInt(Texture, Mode, U.GetSingleValue(), V.GetSingleValue());
		}
		else
		{
			ensure(Texture.GetMin() <= Texture.GetMax());
			return { Texture.GetMin(), Texture.GetMax() };
		}
	}

	inline void FindColorsAlphas(
		const int Threshold,
		const TArray<FColor>& Colors,
		const FColor& Color,
		v_flt OutAlphas[])
	{
		for (int32 Index = 0; Index < Colors.Num(); Index++)
		{
			// Ignore alpha
			auto& A = Color;
			auto& B = Colors[Index];
			const int32 DiffR = int32(A.R) - int32(B.R);
			const int32 DiffG = int32(A.G) - int32(B.G);
			const int32 DiffB = int32(A.B) - int32(B.B);
			const int32 Value = FMath::Max3(FMath::Abs(DiffR), FMath::Abs(DiffG), FMath::Abs(DiffB));
			OutAlphas[Index] = Value <= Threshold ? 1 : 0;
		}
	}
	template<typename T>
	inline void FindColorsLerpedAlphas(
		const int Threshold,
		const TArray<FColor>& Colors,
		const TVoxelTexture<FColor>& Texture,
		const v_flt X,
		const v_flt Y,
		T SetAlpha)
	{
		const int32 MinX = FMath::FloorToInt(X);
		const int32 MinY = FMath::FloorToInt(Y);

		const int32 MaxX = FMath::CeilToInt(X);
		const int32 MaxY = FMath::CeilToInt(Y);

		const v_flt AlphaX = X - MinX;
		const v_flt AlphaY = Y - MinY;

		check(Colors.Num() < 256);
		v_flt MinXMinYAlphas[256];
		v_flt MaxXMinYAlphas[256];
		v_flt MinXMaxYAlphas[256];
		v_flt MaxXMaxYAlphas[256];

		FVoxelNodeFunctions::FindColorsAlphas(Threshold, Colors, Texture.SampleRaw(MinX, MinY, EVoxelSamplerMode::Clamp), MinXMinYAlphas);
		FVoxelNodeFunctions::FindColorsAlphas(Threshold, Colors, Texture.SampleRaw(MaxX, MinY, EVoxelSamplerMode::Clamp), MaxXMinYAlphas);
		FVoxelNodeFunctions::FindColorsAlphas(Threshold, Colors, Texture.SampleRaw(MinX, MaxY, EVoxelSamplerMode::Clamp), MinXMaxYAlphas);
		FVoxelNodeFunctions::FindColorsAlphas(Threshold, Colors, Texture.SampleRaw(MaxX, MaxY, EVoxelSamplerMode::Clamp), MaxXMaxYAlphas);

		for (int32 Index = 0; Index < Colors.Num(); Index++)
		{
			SetAlpha(Index, FVoxelUtilities::BilinearInterpolation(
				MinXMinYAlphas[Index],
				MaxXMinYAlphas[Index],
				MinXMaxYAlphas[Index],
				MaxXMaxYAlphas[Index],
				AlphaX,
				AlphaY));
		}
	}

	inline void GlobalToLocal(v_flt InX, v_flt InY, v_flt InZ, v_flt& OutX, v_flt& OutY, v_flt& OutZ, const FVoxelContext& Context)
	{
		if (Context.bHasCustomTransform)
		{
			const FVector P = Context.LocalToWorld.InverseTransformPosition(FVector(InX, InY, InZ));
			OutX = P.X;
			OutY = P.Y;
			OutZ = P.Z;
		}
		else
		{
			OutX = InX;
			OutY = InY;
			OutZ = InZ;
		}
	}
	inline void LocalToGlobal(v_flt InX, v_flt InY, v_flt InZ, v_flt& OutX, v_flt& OutY, v_flt& OutZ, const FVoxelContext& Context)
	{
		if (Context.bHasCustomTransform)
		{
			const FVector P = Context.LocalToWorld.TransformPosition(FVector(InX, InY, InZ));
			OutX = P.X;
			OutY = P.Y;
			OutZ = P.Z;
		}
		else
		{
			OutX = InX;
			OutY = InY;
			OutZ = InZ;
		}
	}
	inline void GlobalToLocal(
		TVoxelRange<v_flt> InX, TVoxelRange<v_flt> InY, TVoxelRange<v_flt> InZ,
		TVoxelRange<v_flt>& OutX, TVoxelRange<v_flt>& OutY, TVoxelRange<v_flt>& OutZ, const FVoxelContextRange& Context)
	{
		if (Context.bHasCustomTransform)
		{
			OutX = TVoxelRange<v_flt>::Infinite();
			OutY = TVoxelRange<v_flt>::Infinite();
			OutZ = TVoxelRange<v_flt>::Infinite();
		}
		else
		{
			OutX = InX;
			OutY = InY;
			OutZ = InZ;
		}
	}
	inline void LocalToGlobal(
		TVoxelRange<v_flt> InX, TVoxelRange<v_flt> InY, TVoxelRange<v_flt> InZ,
		TVoxelRange<v_flt>& OutX, TVoxelRange<v_flt>& OutY, TVoxelRange<v_flt>& OutZ, const FVoxelContextRange& Context)
	{
		if (Context.bHasCustomTransform)
		{
			OutX = TVoxelRange<v_flt>::Infinite();
			OutY = TVoxelRange<v_flt>::Infinite();
			OutZ = TVoxelRange<v_flt>::Infinite();
		}
		else
		{
			OutX = InX;
			OutY = InY;
			OutZ = InZ;
		}
	}

	inline void TransformVector(v_flt InX, v_flt InY, v_flt InZ, v_flt& OutX, v_flt& OutY, v_flt& OutZ, const FVoxelContext& Context)
	{
		if (Context.bHasCustomTransform)
		{
			const FVector P = Context.LocalToWorld.TransformVector(FVector(InX, InY, InZ));
			OutX = P.X;
			OutY = P.Y;
			OutZ = P.Z;
		}
		else
		{
			OutX = InX;
			OutY = InY;
			OutZ = InZ;
		}
	}
	inline void InverseTransformVector(v_flt InX, v_flt InY, v_flt InZ, v_flt& OutX, v_flt& OutY, v_flt& OutZ, const FVoxelContext& Context)
	{
		if (Context.bHasCustomTransform)
		{
			const FVector P = Context.LocalToWorld.InverseTransformVector(FVector(InX, InY, InZ));
			OutX = P.X;
			OutY = P.Y;
			OutZ = P.Z;
		}
		else
		{
			OutX = InX;
			OutY = InY;
			OutZ = InZ;
		}
	}
	
	inline void TransformVector(
		TVoxelRange<v_flt> InX, TVoxelRange<v_flt> InY, TVoxelRange<v_flt> InZ,
		TVoxelRange<v_flt>& OutX, TVoxelRange<v_flt>& OutY, TVoxelRange<v_flt>& OutZ, const FVoxelContextRange& Context)
	{
		if (Context.bHasCustomTransform)
		{
			const FVector Scale = Context.LocalToWorld.GetScale3D();
			const TVoxelRange<v_flt> ScaleRange{ Scale.GetMin(), Scale.GetMax() };
			const auto Result = TVoxelRange<v_flt>::Union(InX * ScaleRange, InY * ScaleRange, InZ * ScaleRange);
			
			OutX = Result;
			OutY = Result;
			OutZ = Result;
		}
		else
		{
			OutX = InX;
			OutY = InY;
			OutZ = InZ;
		}
	}
	inline void InverseTransformVector(
		TVoxelRange<v_flt> InX, TVoxelRange<v_flt> InY, TVoxelRange<v_flt> InZ,
		TVoxelRange<v_flt>& OutX, TVoxelRange<v_flt>& OutY, TVoxelRange<v_flt>& OutZ, const FVoxelContextRange& Context)
	{
		if (Context.bHasCustomTransform)
		{
			const FVector Scale = FVector(1.f) / Context.LocalToWorld.GetScale3D();
			const TVoxelRange<v_flt> ScaleRange{ Scale.GetMin(), Scale.GetMax() };
			const auto Result = TVoxelRange<v_flt>::Union(InX * ScaleRange, InY * ScaleRange, InZ * ScaleRange);
			
			OutX = Result;
			OutY = Result;
			OutZ = Result;
		}
		else
		{
			OutX = InX;
			OutY = InY;
			OutZ = InZ;
		}
	}

	inline FColor ColorFromMaterial(const FVoxelMaterial& Material)
	{
		return Material.GetColor();
	}
	inline FVoxelColorRange ColorFromMaterial(const FVoxelMaterialRange& Material)
	{
		return {};
	}

	inline int32 SingleIndexFromMaterial(const FVoxelMaterial& Material)
	{
		return Material.GetSingleIndex();
	}
	inline TVoxelRange<int32> SingleIndexFromMaterial(const FVoxelMaterialRange& Material)
	{
		return { 0, 255 };
	}

	inline void GetUVChannelFromMaterial(FVoxelMaterial Material, int32 Channel, v_flt& U, v_flt& V)
	{
		U = Material.GetU_AsFloat(Channel);
		V = Material.GetV_AsFloat(Channel);
	}
	inline void GetUVChannelFromMaterial(FVoxelMaterialRange Material, TVoxelRange<int32> Channel, TVoxelRange<v_flt>& U, TVoxelRange<v_flt>& V)
	{
		U = { 0.f, 1.f };
		V = { 0.f, 1.f };
	}

	VOXELGRAPH_API v_flt GetPreviousGeneratorValue(
		v_flt X, v_flt Y, v_flt Z,
		const FVoxelContext& Context,
		const FVoxelGeneratorInstance* DefaultGenerator);
	VOXELGRAPH_API TVoxelRange<v_flt> GetPreviousGeneratorValue(
		TVoxelRange<v_flt> X,
		TVoxelRange<v_flt> Y,
		TVoxelRange<v_flt> Z,
		const FVoxelContextRange& Context,
		const FVoxelGeneratorInstance* DefaultGenerator);

	VOXELGRAPH_API FVoxelMaterial GetPreviousGeneratorMaterial(
		v_flt X, v_flt Y, v_flt Z,
		const FVoxelContext& Context,
		const FVoxelGeneratorInstance* DefaultGenerator);

	VOXELGRAPH_API v_flt GetPreviousGeneratorCustomOutput(
		const FName& Name,
		v_flt X, v_flt Y, v_flt Z,
		const FVoxelContext& Context,
		const FVoxelGeneratorInstance* DefaultGenerator);
	VOXELGRAPH_API TVoxelRange<v_flt> GetPreviousGeneratorCustomOutput(
		const FName& Name,
		TVoxelRange<v_flt> X,
		TVoxelRange<v_flt> Y,
		TVoxelRange<v_flt> Z,
		const FVoxelContextRange& Context,
		const FVoxelGeneratorInstance* DefaultGenerator);

	VOXELGRAPH_API v_flt GetGeneratorCustomOutput(
		const FVoxelGeneratorInstance& Generator,
		const FName& Name,
		v_flt X, v_flt Y, v_flt Z,
		const FVoxelContext& Context);
	VOXELGRAPH_API TVoxelRange<v_flt> GetGeneratorCustomOutput(
		const FVoxelGeneratorInstance& Generator,
		const FName& Name,
		TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, TVoxelRange<v_flt> Z,
		const FVoxelContextRange& Context);

	inline v_flt Fmod(v_flt X, v_flt Y)
	{
		if (FMath::Abs(Y) <= 1.e-8f)
		{
			return 0.;
		}
		if (TIsSame<v_flt, float>::Value)
		{
			return FMath::Fmod(X, Y);
		}
		else
		{
			return std::fmod(X, Y);
		}
	}

	inline TVoxelRange<v_flt> Fmod(TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y)
	{
		if (X.IsSingleValue() && Y.IsSingleValue())
		{
			return Fmod(X.GetSingleValue(), Y.GetSingleValue());
		}

		const v_flt YMaxAbs = FMath::Max(FMath::Abs(Y.Min), FMath::Abs(Y.Max));
		const TVoxelRange<v_flt> Result = { -YMaxAbs, YMaxAbs };
		if (Result.Contains(X))
		{
			return X;
		}
		else
		{
			return Result;
		}
	}

	inline int32 Mod(int32 X, int32 Y)
	{
		if (Y == 0)
		{
			return 0;
		}
		return X % Y;
	}
	inline TVoxelRange<int32> Mod(TVoxelRange<int32> X, TVoxelRange<int32> Y)
	{
		if (X.IsSingleValue() && Y.IsSingleValue())
		{
			return Mod(X.GetSingleValue(), Y.GetSingleValue());
		}

		const int32 YMaxAbs = FMath::Max(FMath::Abs(Y.Min), FMath::Abs(Y.Max));
		const TVoxelRange<int32> Result = { -YMaxAbs, YMaxAbs };
		if (Result.Contains(X))
		{
			return X;
		}
		else
		{
			return Result;
		}
	}

	inline v_flt OneOverX(v_flt X)
	{
		return 1. / X;
	}
	inline TVoxelRange<v_flt> OneOverX(TVoxelRange<v_flt> X)
	{
		return TVoxelRange<v_flt>(1.) / X;
	}
	
	inline void BreakColor(
		const FColor& Color, 
		int32& OutR, 
		int32& OutG, 
		int32& OutB, 
		int32& OutA)
	{
		OutR = Color.R;
		OutG = Color.G;
		OutB = Color.B;
		OutA = Color.A;
	}
	inline void BreakColor(
		const FVoxelColorRange& Color, 
		TVoxelRange<int32>& OutR, 
		TVoxelRange<int32>& OutG, 
		TVoxelRange<int32>& OutB, 
		TVoxelRange<int32>& OutA)
	{
		OutR = { 0, 255 };
		OutG = { 0, 255 };
		OutB = { 0, 255 };
		OutA = { 0, 255 };
	}
	
	inline void BreakColorFloat(
		const FColor& Color, 
		v_flt& OutR, 
		v_flt& OutG, 
		v_flt& OutB, 
		v_flt& OutA)
	{
		OutR = FVoxelUtilities::UINT8ToFloat(Color.R);
		OutG = FVoxelUtilities::UINT8ToFloat(Color.G);
		OutB = FVoxelUtilities::UINT8ToFloat(Color.B);
		OutA = FVoxelUtilities::UINT8ToFloat(Color.A);
	}
	inline void BreakColorFloat(
		const FVoxelColorRange& Color, 
		TVoxelRange<v_flt>& OutR, 
		TVoxelRange<v_flt>& OutG, 
		TVoxelRange<v_flt>& OutB, 
		TVoxelRange<v_flt>& OutA)
	{
		OutR = { 0, 1 };
		OutG = { 0, 1 };
		OutB = { 0, 1 };
		OutA = { 0, 1 };
	}

	inline FColor MakeColor(int32 R, int32 G, int32 B, int32 A)
	{
		return FColor(
			FMath::Clamp(R, 0, 255),
			FMath::Clamp(G, 0, 255),
			FMath::Clamp(B, 0, 255),
			FMath::Clamp(A, 0, 255));
	}
	inline FVoxelColorRange MakeColor(
		const TVoxelRange<int32>& R,
		const TVoxelRange<int32>& G, 
		const TVoxelRange<int32>& B, 
		const TVoxelRange<int32>& A)
	{
		return {};
	}

	inline FColor MakeColorFloat(v_flt R, v_flt G, v_flt B, v_flt A)
	{
		return FColor(
			FVoxelUtilities::FloatToUINT8(R),
			FVoxelUtilities::FloatToUINT8(G),
			FVoxelUtilities::FloatToUINT8(B),
			FVoxelUtilities::FloatToUINT8(A));
	}
	inline FVoxelColorRange MakeColorFloat(
		const TVoxelRange<v_flt>& R,
		const TVoxelRange<v_flt>& G,
		const TVoxelRange<v_flt>& B, 
		const TVoxelRange<v_flt>& A)
	{
		return {};
	}
	
	inline void RGBToHSV(v_flt R, v_flt G, v_flt B, v_flt& OutH, v_flt& OutS, v_flt& OutV)
	{
		const auto HSVColor = FLinearColor(R, G, B, 0).LinearRGBToHSV();
		OutH = HSVColor.R;
		OutS = HSVColor.G;
		OutV = HSVColor.B;
	}
	inline void RGBToHSV(
		TVoxelRange<v_flt> R, TVoxelRange<v_flt> G, TVoxelRange<v_flt> B,
		TVoxelRange<v_flt>& OutH, TVoxelRange<v_flt>& OutS, TVoxelRange<v_flt>& OutV)
	{
		OutH = { 0, 360 };
		OutS = { 0, 1 };
		OutV = TVoxelRange<v_flt>::Union(R, G, B);
	}
	
	inline void HSVToRGB(v_flt H, v_flt S, v_flt V, v_flt& OutR, v_flt& OutG, v_flt& OutB)
	{
		const auto RGBColor = FLinearColor(H, S, V, 0).HSVToLinearRGB();
		OutR = RGBColor.R;
		OutG = RGBColor.G;
		OutB = RGBColor.B;
	}
	inline void HSVToRGB(
		TVoxelRange<v_flt> H, TVoxelRange<v_flt> S, TVoxelRange<v_flt> V,
		TVoxelRange<v_flt>& OutR, TVoxelRange<v_flt>& OutG, TVoxelRange<v_flt>& OutB)
	{
		OutR = OutG = OutB = TVoxelRange<v_flt>::Union(0, V);
	}

	VOXELGRAPH_API TArray<TVoxelSharedPtr<FVoxelGeneratorInstance>> CreateGeneratorArray(const TArray<FVoxelGeneratorPicker>& Generators);

	VOXELGRAPH_API void ComputeGeneratorsMerge(
		EVoxelMaterialConfig MaterialConfig,
		float Tolerance,
		const TArray<TVoxelSharedPtr<FVoxelGeneratorInstance>>& InInstances,
		const TArray<FName>& FloatOutputsNames,
		const FVoxelContext& Context,
		v_flt X, v_flt Y, v_flt Z,
		int32 Index0, float Alpha0,
		int32 Index1, float Alpha1,
		int32 Index2, float Alpha2,
		int32 Index3, float Alpha3,
		bool bComputeValue, bool bComputeMaterial, const TArray<bool>& ComputeFloatOutputs,
		v_flt& OutValue,
		FVoxelMaterial& OutMaterial,
		TArray<v_flt, TInlineAllocator<128>>& OutFloatOutputs,
		int32& NumGeneratorsQueried);
	
	VOXELGRAPH_API void ComputeGeneratorsMergeRange(
		const TArray<TVoxelSharedPtr<FVoxelGeneratorInstance>>& InInstances,
		const TArray<FName>& FloatOutputsNames,
		const FVoxelContextRange& Context,
		TVoxelRange<v_flt> X,
		TVoxelRange<v_flt> Y,
		TVoxelRange<v_flt> Z,
		bool bComputeValue, const TArray<bool>& ComputeFloatOutputs,
		TVoxelRange<v_flt>& OutValue,
		TArray<TVoxelRange<v_flt>, TInlineAllocator<128>> & OutFloatOutputs,
		TVoxelRange<int32>& NumGeneratorsQueried);

	template<typename T>
	inline T Switch(T A, T B, bool bPickA)
	{
		return bPickA ? A : B;
	}
	template<typename T>
	inline TVoxelRange<T> Switch(TVoxelRange<T> A, TVoxelRange<T> B, FVoxelBoolRange PickA)
	{
		if (!PickA.bCanBeTrue)
		{
			return B;
		}
		if (!PickA.bCanBeFalse)
		{
			return A;
		}
		return TVoxelRange<T>::Union(A, B);
	}
}