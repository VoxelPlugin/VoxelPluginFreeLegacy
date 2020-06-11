// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"

enum class EVoxelDistanceFieldInputType
{
	// Distances to the surface. Unknown distances must be high enough to be cleared by a min with the real distance.
	Distances,
	// Densities, the distance is found by looking at the ratio to neighboring densities
	Densities
};

struct VOXEL_API FVoxelDistanceFieldUtilities
{
public:
	/**
	 * Takes in input distances or densities (only need to be exact near the surface)
	 * and will expand them to find the "exact" distance field
	 *
	 * The result will not be entirely exact: the distance fields will only be propagated using mins, which won't be a "true" distance but should be good enough
	 * Game thread only
	 * 
	 * NOTE: will return the absolute value of the distance!
	 *
	 * @param	NumberOfPasses	Distances will be exact up to this distance from the surface
	 */
	static void ComputeDistanceField_GPU(
		const FIntVector& Size,
		TArray<FFloat16>& InOutData,
		EVoxelDistanceFieldInputType InputType,
		int32 NumberOfPasses);

public:
	template<typename T, typename TLambda>
	static void ConvertDensitiesToDistances(const FIntVector& Size, TArrayView<const T> Densities, TArrayView<float> OutDistances, TLambda GetFloatFromT);

	static void ConvertDensitiesToDistances(const FIntVector& Size, TArrayView<const float> Densities, TArrayView<float> OutDistances);
	static void ConvertDensitiesToDistances(const FIntVector& Size, TArrayView<const FVoxelValue> Densities, TArrayView<float> OutDistances);
	
	// NOTE: since we use the same buffer in input and output, data is written as it's read
	// This will lead to a faster propagation of the distances, although the speed of propagation will likely be biased by the direction
	static void ExpandDistanceField(const FIntVector& Size, TArrayView<float> InOutData);
};