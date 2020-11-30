// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelVector.h"
#include "VoxelIntBox.h"
#include "VoxelWorldInterface.generated.h"

UENUM(BlueprintType)
enum class EVoxelWorldCoordinatesRounding : uint8
{
	RoundToNearest,
	RoundUp,
	RoundDown
};

class IVoxelWorldInterface
{
public:
	virtual ~IVoxelWorldInterface() = default;

	virtual FIntVector GlobalToLocal(const FVector& Position, EVoxelWorldCoordinatesRounding Rounding = EVoxelWorldCoordinatesRounding::RoundToNearest) const = 0;
	virtual FVoxelVector GlobalToLocalFloat(const FVector& Position) const = 0;

	virtual FVector LocalToGlobal(const FIntVector& Position) const = 0;
	virtual FVector LocalToGlobalFloat(const FVoxelVector& Position) const = 0;
	
	virtual FBox LocalToGlobalBounds(const FVoxelIntBox& Bounds) const = 0;
	virtual FVoxelIntBox GlobalToLocalBounds(const FBox& Bounds) const = 0;
};