// Copyright 2020 Phyronnaz

#include "VoxelPlaceableItems/VoxelPlaceableItemsUtilities.h"

#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelData/VoxelData.inl"
#include "VoxelWorld.h"
#include "FastNoise/VoxelFastNoise.inl"

#include "Misc/ScopeExit.h"

void UVoxelPlaceableItemsUtilities::AddWorms(FAddWorm AddWorm, const FVoxelPerlinWormsSettings Settings)
{
	struct FLocalData
	{
		int32 Seed;
		FRandomStream Stream;
		int32 NumWorms = 0;
	};
	static TUniquePtr<FLocalData> LocalData;

	const bool bIsRoot = !LocalData.IsValid();
	ON_SCOPE_EXIT
	{
		if (bIsRoot)
		{
			LocalData.Reset();
		}
	};

	if (bIsRoot)
	{
		LocalData = MakeUnique<FLocalData>();

		LocalData->Seed = Settings.Seed;
		LocalData->Stream = FRandomStream(LocalData->Seed++);
	}

	LocalData->NumWorms++;

	if (LocalData->NumWorms > Settings.MaxWorms)
	{
		return;
	}

	// Noise modules are per call, as they have different seeds
	
	FVoxelFastNoise ModuleX;
	FVoxelFastNoise ModuleY;
	FVoxelFastNoise ModuleZ;
	
	ModuleX.SetSeed(LocalData->Seed++);
	ModuleY.SetSeed(LocalData->Seed++);
	ModuleZ.SetSeed(LocalData->Seed++);

	auto& Stream = LocalData->Stream;

	FVector CurrentPosition = Settings.Start;
	FVector CurrentDir = Settings.Direction.GetSafeNormal();
	for (int32 SegmentIndex = 0; SegmentIndex < Settings.NumSegments; SegmentIndex++)
	{
		const FVector NewPosition = CurrentPosition + CurrentDir * Settings.SegmentLength;
		AddWorm.ExecuteIfBound(CurrentPosition, NewPosition, Settings.Radius);
		CurrentPosition = NewPosition;

		const FVector NoisePosition = Settings.NoiseDirection * Settings.NoiseSegmentLength * SegmentIndex;
		CurrentDir = CurrentDir.RotateAngleAxis(Settings.RotationAmplitude.X * ModuleX.GetSimplex_3D(NoisePosition.X, NoisePosition.Y, NoisePosition.Z, 0.02f), FVector(1, 0, 0));
		CurrentDir = CurrentDir.RotateAngleAxis(Settings.RotationAmplitude.Y * ModuleY.GetSimplex_3D(NoisePosition.X, NoisePosition.Y, NoisePosition.Z, 0.02f), FVector(0, 1, 0));
		CurrentDir = CurrentDir.RotateAngleAxis(Settings.RotationAmplitude.Z * ModuleZ.GetSimplex_3D(NoisePosition.X, NoisePosition.Y, NoisePosition.Z, 0.02f), FVector(0, 0, 1));

		if (Stream.FRand() < Settings.SplitProbability)
		{
			auto NewSettings = Settings;
			NewSettings.Start = CurrentPosition;
			NewSettings.Direction = CurrentDir
				.RotateAngleAxis((Stream.FRand() * 2 - 1) * Settings.RotationAmplitude.X, FVector(1, 0, 0))
				.RotateAngleAxis((Stream.FRand() * 2 - 1) * Settings.RotationAmplitude.Y, FVector(0, 1, 0))
				.RotateAngleAxis((Stream.FRand() * 2 - 1) * Settings.RotationAmplitude.Z, FVector(0, 0, 1));

			NewSettings.NumSegments = FMath::Min<int32>(Settings.BranchMeanSize + (Stream.FRand() * 2 - 1) * Settings.BranchSizeVariation * Settings.BranchMeanSize, Settings.NumSegments - (SegmentIndex + 1));
			NewSettings.SplitProbability *= Settings.SplitProbabilityGain;

			AddWorms(AddWorm, NewSettings);
		}
	}
}