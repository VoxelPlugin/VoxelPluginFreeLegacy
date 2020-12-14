// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelInterval.generated.h"

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelFloatInterval
#if CPP
	: FFloatInterval
#endif
{
	GENERATED_BODY()

	using FFloatInterval::FFloatInterval;
	
#if !CPP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float Min = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float Max = 0;
#endif
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelInt32Interval
#if CPP
	: FInt32Interval
#endif
{
	GENERATED_BODY()

	using FInt32Interval::FInt32Interval;
	
#if !CPP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	int32 Min = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	int32 Max = 0;
#endif
};