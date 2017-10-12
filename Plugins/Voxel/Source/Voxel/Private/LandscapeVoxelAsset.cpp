// Copyright 2017 Phyronnaz

#pragma once
#include "VoxelPrivatePCH.h"
#include "LandscapeComponent.h"
#include "VoxelAssets/LandscapeVoxelAsset.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"

ALandscapeVoxelAsset::ALandscapeVoxelAsset() : Precision(1), ValueMultiplier(1)
{
};

float ALandscapeVoxelAsset::GetDefaultValue(const int X, const int Y, const int Z)
{
	check(0 <= X && X < Size && 0 <= Y && Y < Size);

	if ((Z + Precision) * VoxelSize < Heights[X + Size * Y])
	{
		// If voxel over us is in, we're entirely in
		return -ValueMultiplier;
	}
	else if ((Z - Precision) * VoxelSize > Heights[X + Size * Y])
	{
		// If voxel under us is out, we're entirely out
		return ValueMultiplier;
	}
	else
	{
		float Alpha = (Z * VoxelSize - Heights[X + Size * Y]) / VoxelSize / Precision;

		if (Alpha < 0)
		{
			Alpha *= ValueMultiplier;
		}
		else
		{
			Alpha *= ValueMultiplier;
		}

		return Alpha;
	}
}

FVoxelMaterial ALandscapeVoxelAsset::GetDefaultMaterial(const int X, const int Y, const int Z)
{
	check(0 <= X && X < Size && 0 <= Y && Y < Size);

	// TODO: FColor->FVoxelMaterial
	return FVoxelMaterial(Weights[X + Size * Y]);
}


void ALandscapeVoxelAsset::Init(float InVoxelSize)
{
	VoxelSize = InVoxelSize;
}

int ALandscapeVoxelAsset::GetMinBound(int X, int Y)
{
	return FMath::FloorToInt(Heights[X + Size * Y] / (float)VoxelSize - Precision);
}

int ALandscapeVoxelAsset::GetMaxBound(int X, int Y)
{
	return FMath::CeilToInt(Heights[X + Size * Y] / (float)VoxelSize + Precision);
}
