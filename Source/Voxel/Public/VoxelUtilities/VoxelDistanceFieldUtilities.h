// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelConfigEnums.h"

struct VOXEL_API FVoxelDistanceFieldUtilities
{
public:
	FORCEINLINE static bool IsSurfacePositionValid(const FVector& P)
	{
		return P.X < 1e9;
	}
	FORCEINLINE static FVector MakeInvalidSurfacePosition()
	{
		return FVector(1e9);
	}

public:
	static FColor GetDistanceFieldColor(float Value);

public:
	static void JumpFlood(const FIntVector& Size, TArray<FVector>& InOutPackedPositions, EVoxelComputeDevice Device, bool bMultiThreaded = false, int32 MaxPasses_Debug = -1);
	// Only the InOutDistances sign will be used, not their actual values
	static void GetDistancesFromSurfacePositions(const FIntVector& Size, TArrayView<const FVector> SurfacePositions, TArrayView<float> InOutDistances);
	
public:
	// OutDistances will only have the signs of the values
	// Note: densities need to match Size + 2, so that all neighbors can be queried!
	template<typename T, typename TLambda>
	static void GetSurfacePositionsFromDensities(
		const FIntVector& Size,
		TArrayView<const T> Densities,
		TArrayView<float> OutDistances,
		TArrayView<FVector> OutSurfacePositions,
		TLambda GetFloatFromT);

	static void GetSurfacePositionsFromDensities(const FIntVector& Size, TArrayView<const float> Densities, TArrayView<float> OutDistances, TArrayView<FVector> OutSurfacePositions);
	static void GetSurfacePositionsFromDensities(const FIntVector& Size, TArrayView<const FVoxelValue> Densities, TArrayView<float> OutDistances, TArrayView<FVector> OutSurfacePositions);
	
	static void GetSurfacePositionsFromDensities(const FIntVector& Size, TArrayView<const FVoxelValue> Densities, TArray<float>& OutDistances, TArray<FVector>& OutSurfacePositions);

public:
	// Must be called BEFORE JumpFlood
	// bShrink: if true, will bias towards shrinking the distance field. If false, will bias towards growing it
	// TODO Doesn't work well, signs are leaking & wrongs on the borders
	static void DownSample(
		const FIntVector& Size, 
		TArrayView<const float> InDistances, 
		TArrayView<const FVector> InSurfacePositions,
		TArrayView<float> OutDistances, 
		TArrayView<FVector> OutSurfacePositions,
		int32 Divisor,
		bool bShrink);
	
	static void DownSample(
		FIntVector& Size, 
		TArray<float>& Distances, 
		TArray<FVector>& SurfacePositions,
		int32 Divisor,
		bool bShrink);

private:
	static void JumpFloodStep_CPU(const FIntVector& Size, TArrayView<const FVector> InData, TArrayView<FVector> OutData, int32 Step, bool bMultiThreaded);
};