// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelMathLibrary.h"
#include "VoxelUtilities/VoxelBaseUtilities.h"

FVector UVoxelMathLibrary::GetUnitVectorFromRandom(FVector2D Random)
{
	// From Raytracing Gems, Chapter 16

	// Compute radius r (branchless).
	Random = 2 * Random - 1;
	const float d = 1 - (FMath::Abs(Random.X) + FMath::Abs(Random.Y));
	const float Radius = 1 - FMath::Abs(d);

	// Compute phi in the first quadrant (branchless, except for the division-by-zero test),
	// using sign(random) to map the result to the correct quadrant below
	const float Phi = (Radius == 0) ? 0 : (PI / 4) * ((FMath::Abs(Random.Y) - FMath::Abs(Random.X)) / Radius + 1);
	const float f = Radius * FMath::Sqrt(2 - Radius * Radius);

	FVector Result;
	Result.X = f * FMath::Sign(Random.X) * FMath::Cos(Phi);
	Result.Y = f * FMath::Sign(Random.Y) * FMath::Sin(Phi);
	Result.Z = FMath::Sign(d) * (1 - Radius * Radius);

	ensure(Result.IsUnit());
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelHaltonStream UVoxelMathLibrary::MakeHaltonStream(int32 InitialSeed)
{
	return { InitialSeed, uint32(FMath::Abs(InitialSeed)) };
}

void UVoxelMathLibrary::ResetHaltonStream(const FVoxelHaltonStream& Stream)
{
	Stream.Seed = Stream.InitialSeed;
}

float UVoxelMathLibrary::GetHalton1D(const FVoxelHaltonStream& Stream)
{
	const float Value = FVoxelUtilities::Halton<2>(Stream.Seed);
	Stream.Seed++;
	return Value;
}

FVector2D UVoxelMathLibrary::GetHalton2D(const FVoxelHaltonStream& Stream)
{
	FVector2D Value;
	Value.X = FVoxelUtilities::Halton<2>(Stream.Seed);
	Value.Y = FVoxelUtilities::Halton<3>(Stream.Seed);
	Stream.Seed++;
	return Value;
}

FVector UVoxelMathLibrary::GetHalton3D(const FVoxelHaltonStream& Stream)
{
	FVector Value;
	Value.X = FVoxelUtilities::Halton<2>(Stream.Seed);
	Value.Y = FVoxelUtilities::Halton<3>(Stream.Seed);
	Value.Z = FVoxelUtilities::Halton<5>(Stream.Seed);
	Stream.Seed++;
	return Value;
}