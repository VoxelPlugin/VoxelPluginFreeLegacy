// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/VoxelDataTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelShaders/VoxelDistanceFieldShader.h"

template<typename T1, typename T2>
void UVoxelDataTools::MergeDistanceFieldImpl(FVoxelData& Data, const FIntBox& Bounds, T1 GetSDF, T2 MergeSDF, float MaxDistance, bool bMultiThreaded)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
	
	const auto Get = [&](auto& Array, auto Index) -> auto&
	{
#if VOXEL_DEBUG
		return Array[Index];
#else
			return Array.GetData()[Index];
#endif
	};

	const FIntVector Size = Bounds.Size();

	TArray<FFloat16> DistanceField;
	DistanceField.SetNumUninitialized(Bounds.Count());

	const TArray<FVoxelValue> Values = Data.ParallelGet<FVoxelValue>(Bounds, !bMultiThreaded);

	{
		VOXEL_SCOPE_COUNTER("Expand values to floats");
		ParallelFor(DistanceField.Num(), [&](int32 Index)
		{
			Get(DistanceField, Index) = Get(Values, Index).ToFloat();
		}, !bMultiThreaded);
	}

	{
		VOXEL_SCOPE_COUNTER("Compute distance field");
		auto DataPtr = MakeVoxelShared<TArray<FFloat16>>(MoveTemp(DistanceField));
		DataPtr->SetNum(Size.X * Size.Y * Size.Z);
		auto Helper = MakeVoxelShared<FVoxelDistanceFieldShaderHelper>();
		Helper->StartCompute(Size, DataPtr, FMath::CeilToInt(MaxDistance), true);
		Helper->WaitForCompletion();
		DistanceField = MoveTemp(*DataPtr);
	}

	{
		VOXEL_SCOPE_COUNTER("Merge SDFs");

		// Distance on boundaries is invalid, don't consider these
		ParallelFor(Size.X - 2, [&](int32 XIndex)
		{
			const int32 X = XIndex + 1;
			for (int32 Y = 1; Y < Size.Y - 1; Y++)
			{
				for (int32 Z = 1; Z < Size.Z - 1; Z++)
				{
					const int32 Index = X + Y * Size.X + Z * Size.X * Size.Y;
					FFloat16& Distance = Get(DistanceField, Index);
					const float SDF = float(Distance) * Values[Index].Sign();
					const float OtherSDF = GetSDF(Bounds.Min.X + X, Bounds.Min.Y + Y, Bounds.Min.Z + Z);
					Distance = MergeSDF(SDF, OtherSDF);
				}
			}
		}, !bMultiThreaded);
	}

	Data.Set<FVoxelValue>(Bounds.Extend(-1), [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
	{
		checkVoxelSlow(Bounds.Contains(X, Y, Z));
		X -= Bounds.Min.X;
		Y -= Bounds.Min.Y;
		Z -= Bounds.Min.Z;
		checkVoxelSlow(0 < X && X < Size.X);
		checkVoxelSlow(0 < Y && Y < Size.Y);
		checkVoxelSlow(0 < Z && Z < Size.Z);

		Value = FVoxelValue(float(Get(DistanceField, X + Size.X * Y + Size.X * Size.Y * Z)));
	});
}