// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Curves/RichCurve.h"

namespace FVoxelRichCurveUtilities
{
	inline float BezierInterp(float P0, float P1, float P2, float P3, float Alpha)
	{
		const float P01 = FMath::Lerp(P0, P1, Alpha);
		const float P12 = FMath::Lerp(P1, P2, Alpha);
		const float P23 = FMath::Lerp(P2, P3, Alpha);
		const float P012 = FMath::Lerp(P01, P12, Alpha);
		const float P123 = FMath::Lerp(P12, P23, Alpha);
		const float P0123 = FMath::Lerp(P012, P123, Alpha);

		return P0123;
	}

	inline float EvalForTwoKeys(const FRichCurveKey& Key1, const FRichCurveKey& Key2, const float InTime)
	{
		const float Diff = Key2.Time - Key1.Time;

		if (Diff > 0.f && Key1.InterpMode != RCIM_Constant)
		{
			const float Alpha = (InTime - Key1.Time) / Diff;
			const float P0 = Key1.Value;
			const float P3 = Key2.Value;

			if (Key1.InterpMode == RCIM_Linear)
			{
				return FMath::Lerp(P0, P3, Alpha);
			}
			else
			{
				const float OneThird = 1.0f / 3.0f;
				const float P1 = P0 + (Key1.LeaveTangent * Diff * OneThird);
				const float P2 = P3 - (Key2.ArriveTangent * Diff * OneThird);

				return BezierInterp(P0, P1, P2, P3, Alpha);
			}
		}
		else
		{
			return Key1.Value;
		}
	}

	inline float Eval(const FRichCurve& Curve, float InTime) // Inline + doesn't trigger a stat on every call
	{
		constexpr float InDefaultValue = 0.f;
		const int32 NumKeys = Curve.Keys.Num();

		// Remap time if extrapolation is present and compute offset value to use if cycling 
		float CycleValueOffset = 0;
		Curve.RemapTimeValue(InTime, CycleValueOffset);

		// If the default value hasn't been initialized, use the incoming default value
		float InterpVal = Curve.DefaultValue == MAX_flt ? InDefaultValue : Curve.DefaultValue;

		if (NumKeys == 0)
		{
			// If no keys in curve, return the Default value.
		}
		else if (NumKeys < 2 || (InTime <= Curve.Keys[0].Time))
		{
			if (Curve.PreInfinityExtrap == RCCE_Linear && NumKeys > 1)
			{
				const float DT = Curve.Keys[1].Time - Curve.Keys[0].Time;

				if (FMath::IsNearlyZero(DT))
				{
					InterpVal = Curve.Keys[0].Value;
				}
				else
				{
					const float DV = Curve.Keys[1].Value - Curve.Keys[0].Value;
					const float Slope = DV / DT;

					InterpVal = Slope * (InTime - Curve.Keys[0].Time) + Curve.Keys[0].Value;
				}
			}
			else
			{
				// Otherwise if constant or in a cycle or oscillate, always use the first key value
				InterpVal = Curve.Keys[0].Value;
			}
		}
		else if (InTime < Curve.Keys[NumKeys - 1].Time)
		{
			// perform a lower bound to get the second of the interpolation nodes
			int32 first = 1;
			int32 last = NumKeys - 1;
			int32 count = last - first;

			while (count > 0)
			{
				int32 step = count / 2;
				int32 middle = first + step;

				if (InTime >= Curve.Keys[middle].Time)
				{
					first = middle + 1;
					count -= step + 1;
				}
				else
				{
					count = step;
				}
			}

			InterpVal = EvalForTwoKeys(Curve.Keys[first - 1], Curve.Keys[first], InTime);
		}
		else
		{
			if (Curve.PostInfinityExtrap == RCCE_Linear)
			{
				float DT = Curve.Keys[NumKeys - 2].Time - Curve.Keys[NumKeys - 1].Time;

				if (FMath::IsNearlyZero(DT))
				{
					InterpVal = Curve.Keys[NumKeys - 1].Value;
				}
				else
				{
					float DV = Curve.Keys[NumKeys - 2].Value - Curve.Keys[NumKeys - 1].Value;
					float Slope = DV / DT;

					InterpVal = Slope * (InTime - Curve.Keys[NumKeys - 1].Time) + Curve.Keys[NumKeys - 1].Value;
				}
			}
			else
			{
				// Otherwise if constant or in a cycle or oscillate, always use the last key value
				InterpVal = Curve.Keys[NumKeys - 1].Value;
			}
		}

		return InterpVal + CycleValueOffset;
	}
}