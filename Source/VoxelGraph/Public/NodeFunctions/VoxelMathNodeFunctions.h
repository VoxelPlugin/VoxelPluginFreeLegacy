// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelRange.h"

namespace FVoxelMathNodeFunctions
{
	FORCEINLINE void InverseTransformPositionXZ(
		v_flt X_X, v_flt X_Y, v_flt X_Z,
		v_flt Z_X, v_flt Z_Y, v_flt Z_Z,
		v_flt X, v_flt Y, v_flt Z,
		v_flt& OutX, v_flt& OutY, v_flt& OutZ)
	{
		const FVector BaseX = FVoxelVector(X_X, X_Y, X_Z).GetSafeNormal().ToFloat();
        FVector BaseZ = FVoxelVector(Z_X, Z_Y, Z_Z).GetSafeNormal().ToFloat();
        const FVector BaseY = BaseZ ^ BaseX;
        BaseZ = BaseX ^ BaseY;

		if (BaseX.IsNearlyZero() || BaseY.IsNearlyZero() || BaseZ.IsNearlyZero())
		{
			OutX = 0;
			OutY = 0;
			OutZ = 0;
			return;
		}

		const FMatrix Matrix(BaseX, BaseY, BaseZ, FVector::ZeroVector);
        const FVector Result = Matrix.InverseTransformPosition(FVoxelVector(X, Y, Z).ToFloat());
		
		OutX = Result.X;
		OutY = Result.Y;
		OutZ = Result.Z;
	}
	FORCEINLINE void InverseTransformPositionXZ(
		TVoxelRange<v_flt> X_X, TVoxelRange<v_flt> X_Y, TVoxelRange<v_flt> X_Z,
		TVoxelRange<v_flt> Z_X, TVoxelRange<v_flt> Z_Y, TVoxelRange<v_flt> Z_Z,
		TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, TVoxelRange<v_flt> Z,
		TVoxelRange<v_flt>& OutX, TVoxelRange<v_flt>& OutY, TVoxelRange<v_flt>& OutZ)
	{
		if (!X_X.IsSingleValue() || !X_Y.IsSingleValue() || !X_Z.IsSingleValue() ||
			!Z_X.IsSingleValue() || !Z_Y.IsSingleValue() || !Z_Z.IsSingleValue())
		{
			FVoxelRangeFailStatus::Get().Warning(TEXT("range analysis needs a fixed basis to work"));
			OutX = TVoxelRange<v_flt>::Infinite();
			OutY = TVoxelRange<v_flt>::Infinite();
			OutZ = TVoxelRange<v_flt>::Infinite();
			return;
		}

		const FVector BaseX = FVoxelVector(X_X.GetSingleValue(), X_Y.GetSingleValue(), X_Z.GetSingleValue()).GetSafeNormal().ToFloat();
        FVector BaseZ = FVoxelVector(Z_X.GetSingleValue(), Z_Y.GetSingleValue(), Z_Z.GetSingleValue()).GetSafeNormal().ToFloat();
        const FVector BaseY = BaseZ ^ BaseX;
        BaseZ = BaseX ^ BaseY;

		if (BaseX.IsNearlyZero() || BaseY.IsNearlyZero() || BaseZ.IsNearlyZero())
		{
			OutX = 0;
			OutY = 0;
			OutZ = 0;
			return;
		}
		
		const FMatrix Matrix = FMatrix(BaseX, BaseY, BaseZ, FVector::ZeroVector).InverseFast();

		bool bOutSet = false;

		const auto Check = [&](v_flt InX, v_flt InY, v_flt InZ)
		{
			const FVector Result = Matrix.TransformPosition(FVoxelVector(InX, InY, InZ).ToFloat());

			if (bOutSet)
			{
				OutX = TVoxelRange<v_flt>::Union(OutX, Result.X);
				OutY = TVoxelRange<v_flt>::Union(OutY, Result.Y);
				OutZ = TVoxelRange<v_flt>::Union(OutZ, Result.Z);
			}
			else
			{
				OutX = Result.X;
				OutY = Result.Y;
				OutZ = Result.Z;
				bOutSet = true;
			}
		};

		Check(X.Min, Y.Min, Z.Min);
		Check(X.Max, Y.Min, Z.Min);
		Check(X.Min, Y.Max, Z.Min);
		Check(X.Max, Y.Max, Z.Min);
		Check(X.Min, Y.Min, Z.Max);
		Check(X.Max, Y.Min, Z.Max);
		Check(X.Min, Y.Max, Z.Max);
		Check(X.Max, Y.Max, Z.Max);
	}

	template<typename TInArray, typename TOutArray>
	FORCEINLINE void HeightSplit(v_flt Height, const TInArray& Inputs, TOutArray& Outputs)
	{
		checkVoxelSlow(Inputs.Num() % 2 == 0);
		checkVoxelSlow(Outputs.Num() == Inputs.Num() / 2 + 1);

		const int32 NumSplits = Inputs.Num() / 2;

		const auto GetHeight = [&](int32 Split) { return Inputs[2 * Split]; };
		const auto GetFalloff = [&](int32 Split) { return Inputs[2 * Split + 1]; };

		for (int32 Layer = 0; Layer < NumSplits + 1; Layer++)
		{
			float Strength = 1;

			// Lower bound
			if (Layer != 0)
			{
				const v_flt Bound = GetHeight(Layer - 1);
				const v_flt Falloff = GetFalloff(Layer - 1);

				Strength = FMath::Min(Strength, FMath::SmoothStep(Bound - Falloff, Bound + Falloff, Height));
			}

			// Upper bound
			if (Layer != NumSplits)
			{
				const v_flt Bound = GetHeight(Layer);
				const v_flt Falloff = GetFalloff(Layer);

				Strength = FMath::Min(Strength, 1 - FMath::SmoothStep(Bound - Falloff, Bound + Falloff, Height));
			}

			ensureVoxelSlow(0 <= Strength);
			Outputs[Layer] = Strength;
		}
	}
	template<typename TInArray, typename TOutArray>
	FORCEINLINE void HeightSplit(TVoxelRange<v_flt> Height, const TInArray& Inputs, TOutArray& Outputs)
	{
		for (auto& It : Outputs)
		{
			It = TVoxelRange<v_flt>(0, 1);
		}
	}
}