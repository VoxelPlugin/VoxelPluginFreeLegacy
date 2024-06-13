// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/VoxelDataTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelUtilities/VoxelDistanceFieldUtilities.h"

template<typename T1, typename T2, typename T3>
void UVoxelDataTools::MergeDistanceFieldImpl(
	FVoxelData& Data, 
	const FVoxelIntBox& Bounds, 
	T1 GetSDF, 
	T2 MergeSDF, 
	bool bMultiThreaded,
	bool bSetMaterials,
	T3 GetMaterial)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());

	const FIntVector Size = Bounds.Size();

	const TArray<FVoxelValue> Values = Data.ParallelGet<FVoxelValue>(Bounds.Extend(1) /* See GetSurfacePositionsFromDensities */, !bMultiThreaded);

	TArray<float> Distances;
	TArray<FVector3f> SurfacePositions;
	FVoxelDistanceFieldUtilities::GetSurfacePositionsFromDensities(Size, Values, Distances, SurfacePositions);
	FVoxelDistanceFieldUtilities::JumpFlood(Size, SurfacePositions);
	FVoxelDistanceFieldUtilities::GetDistancesFromSurfacePositions(Size, SurfacePositions, Distances);

	FVoxelDebug::Broadcast("Values", Bounds.Size(), Data.Get<FVoxelValue>(Bounds));
	FVoxelDebug::Broadcast("Distances", Bounds.Size(), Distances);
	
	const auto Set = [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value, FVoxelMaterial& Material, auto bSetMaterials_Static)
	{
		checkVoxelSlow(Bounds.Contains(X, Y, Z));
		
		const float OtherSDF = GetSDF(X, Y, Z);
		const float OldSDF = FVoxelUtilities::Get3D(Distances, Size, X, Y, Z, Bounds.Min);
		const float NewSDF = MergeSDF(OldSDF, OtherSDF);

		Value = FVoxelValue(NewSDF);

		if (bSetMaterials_Static)
		{
			Material = GetMaterial(OldSDF, NewSDF, Material);
		}
	};

	if (bSetMaterials)
	{
		Data.ParallelSet<FVoxelValue, FVoxelMaterial>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value, FVoxelMaterial& Material)
		{
			Set(X, Y, Z, Value, Material, FVoxelUtilities::FTrueType());
		}, !bMultiThreaded);
	}
	else
	{
		// TODO optional
		// TODO check surface position is in bounds
		Data.ParallelSet<FVoxelMaterial>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelMaterial& Material)
		{
			const FVector3f SurfacePosition = FVoxelUtilities::Get3D(SurfacePositions, Size, X, Y, Z, Bounds.Min);
			const auto Result = FindClosestNonEmptyVoxelImpl(Data, FVoxelVector(Bounds.Min) + FVector(SurfacePosition), true);

			if (Result.bSuccess)
			{
				Material = Result.Material;
			}
		}, !bMultiThreaded);
		Data.ParallelSet<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
		{
			FVoxelMaterial Material;
			Set(X, Y, Z, Value, Material, FVoxelUtilities::FFalseType());
		}, !bMultiThreaded);
	}
}