// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FVoxelProcMeshTangent
{
	FVector TangentX = FVector::RightVector;
	bool bFlipTangentY = false;

	FVoxelProcMeshTangent() = default;
	FVoxelProcMeshTangent(float X, float Y, float Z)
		: TangentX(X, Y, Z)
		, bFlipTangentY(false)
	{
	}
	FVoxelProcMeshTangent(FVector InTangentX, bool bInFlipTangentY)
		: TangentX(InTangentX)
		, bFlipTangentY(bInFlipTangentY)
	{
	}

	FVector GetY(const FVector& Normal) const
	{
		return (Normal ^ TangentX) * (bFlipTangentY ? -1 : 1);
	}
};