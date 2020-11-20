// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

struct FVoxelProcMeshTangent
{
	FVector TangentX;
	bool bFlipTangentY;

	FVoxelProcMeshTangent() = default;
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