// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelTransform.h"
#include "VoxelCoordinatesProvider.generated.h"

UENUM(BlueprintType)
enum class EVoxelWorldCoordinatesRounding : uint8
{
	RoundToNearest,
	RoundUp,
	RoundDown
};

class VOXEL_API FVoxelCoordinatesProvider
{
public:
	FVoxelCoordinatesProvider() = default;
	virtual ~FVoxelCoordinatesProvider() = default;
	
	FVoxelVector GlobalToLocalFloat(const FVoxelVector& Position) const;
	FVoxelVector LocalToGlobalFloat(const FVoxelVector& Position) const;

	FIntVector GlobalToLocal(const FVoxelVector& Position, EVoxelWorldCoordinatesRounding Rounding = EVoxelWorldCoordinatesRounding::RoundToNearest) const;
	FVoxelVector LocalToGlobal(const FIntVector& Position) const;

	FBox LocalToGlobalBounds(const FVoxelIntBox& Bounds) const;
	FVoxelIntBox GlobalToLocalBounds(const FBox& Bounds) const;

public:
	//~ Begin FVoxelCoordinatesProvider Interface
	virtual FVoxelDoubleTransform GetVoxelTransform() const = 0;
	virtual v_flt GetVoxelSize() const = 0;
	//~ End FVoxelCoordinatesProvider Interface	
};