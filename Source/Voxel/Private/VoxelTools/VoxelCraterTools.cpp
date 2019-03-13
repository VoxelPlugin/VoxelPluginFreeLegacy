// Copyright 2019 Phyronnaz

#include "VoxelTools/VoxelCraterTools.h"
#include "VoxelTools/VoxelToolsHelpers.h"
#include "VoxelTools/VoxelLatentActionHelpers.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "FastNoise.h"

inline FIntBox GetCraterBounds(float Radius, const FIntVector& Position)
{
	FIntVector R(FMath::CeilToInt(Radius) + 3);
	return FIntBox(Position - R, Position + R);
}

template<bool bUseSampleRate>
inline void AddCraterHelperImpl(FVoxelData& Data, const FIntBox& Bounds, const FIntVector& Position, float Radius, float SampleRate, TArray<FIntVector>& OutPositions)
{
	FastNoise Noise;
		
	const float SquaredRadiusPlus2 = FMath::Square(Radius + 2);
	const float SquaredRadiusMinus2 = FMath::Square(Radius - 2);
	Data.SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& OldValue)
	{
		float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();

		if (SquaredDistance <= SquaredRadiusPlus2)
		{
			float WhiteNoise = 0;
			if (SquaredRadiusMinus2 <= SquaredDistance)
			{
				WhiteNoise = Noise.GetWhiteNoise(X, Y, Z);
			}

			float Distance = FMath::Sqrt(SquaredDistance);

			FVoxelValue NewValue = FMath::Clamp(Radius - Distance + WhiteNoise, -2.f, 2.f) / 2;

			if (NewValue.IsEmpty() || FVoxelValue::HaveSameSign(OldValue, NewValue))
			{
				if (bUseSampleRate && !OldValue.IsEmpty() && (SampleRate >= 1 || FMath::FRand() < SampleRate))
				{
					OutPositions.Emplace(X, Y, Z);
				}
				OldValue = NewValue;
			}
		}
	});
}

inline void AddCraterHelper(FVoxelData& Data, const FIntBox& Bounds, const FIntVector& Position, float Radius, float SampleRate, TArray<FIntVector>& OutPositions)
{
	if (SampleRate > 0)
	{
		AddCraterHelperImpl<true>(Data, Bounds, Position, Radius, SampleRate, OutPositions);
	}
	else
	{
		AddCraterHelperImpl<false>(Data, Bounds, Position, Radius, SampleRate, OutPositions);
	}
}

///////////////////////////////////////////////////////////////////////////////

DECLARE_CYCLE_STAT(TEXT("UVoxelCraterTools::AddCrater"), STAT_UVoxelCraterTools_AddCrater, STATGROUP_Voxel);

bool UVoxelCraterTools::AddCrater(
		AVoxelWorld* World, 
		FIntVector Position, 
		float Radius, 
		float PositionsSampleRate, 
		TArray<FIntVector>& SampledPositions,
		bool bUpdateRender, 
		bool bAllowFailure)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelCraterTools_AddCrater);
	CHECK_VOXELWORLD_IS_CREATED();
	
	SampledPositions.Reset();
	auto Bounds = GetCraterBounds(Radius, Position);

	return FVoxelToolsHelpers::EditToolsHelper<EVoxelLockType::ReadWrite>(__FUNCTION__, World, Bounds, bUpdateRender, bAllowFailure, [&](FVoxelData& Data)
	{
		AddCraterHelper(Data, Bounds, Position, Radius, PositionsSampleRate, SampledPositions);
	});
}

void UVoxelCraterTools::AddCraterAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		const FIntVector& Position, 
		float Radius, 
		float PositionsSampleRate, 
		TArray<FIntVector>& SampledPositions,
		bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	SampledPositions.Reset();
	auto Bounds = GetCraterBounds(Radius, Position);

	FVoxelLatentActionHelpers::AsyncHelperWithValue(WorldContextObject, LatentInfo, __FUNCTION__, World, SampledPositions, [Bounds, Position, Radius, PositionsSampleRate](FVoxelData& Data, TArray<FIntVector>& OutSampledPositions)
	{
		FVoxelReadWriteScopeLock Lock(Data, Bounds, __FUNCTION__);
		AddCraterHelper(Data, Bounds, Position, Radius, PositionsSampleRate, OutSampledPositions);
	}, bUpdateRender, Bounds);
}
