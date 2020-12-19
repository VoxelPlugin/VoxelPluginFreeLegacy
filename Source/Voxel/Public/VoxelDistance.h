// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelDistance.generated.h"

UENUM(BlueprintType)
enum class EVoxelDistanceType : uint8
{
	Voxels,
	Centimeters
};

USTRUCT(BlueprintType)
struct FVoxelDistance
{
	GENERATED_BODY()

	FVoxelDistance() = default;
	
	static FVoxelDistance Voxels(float Value) { return { EVoxelDistanceType::Voxels, Value }; }
	static FVoxelDistance Centimeters(float Value) { return { EVoxelDistanceType::Centimeters, Value }; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	EVoxelDistanceType Type = EVoxelDistanceType::Voxels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (ClampMin = 0))
	float Distance = 0.f;

	float GetInVoxels(float VoxelSize) const
	{
		return Type == EVoxelDistanceType::Voxels ? Distance : (Distance / VoxelSize);
	}

	friend bool operator==(const FVoxelDistance& Lhs, const FVoxelDistance& Rhs)
	{
		return Lhs.Type == Rhs.Type
			&& Lhs.Distance == Rhs.Distance;
	}
};
