// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"

struct FVoxelValueDiff
{
	uint64 Id;
	uint32 Index;
	float Value;

	FVoxelValueDiff() = default;
	FVoxelValueDiff(uint64 Id, uint32 Index, float Value) : Id(Id), Index(Index), Value(Value) {}
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, FVoxelValueDiff& ValueDiff)
{
	Ar << ValueDiff.Id;
	Ar << ValueDiff.Index;
	Ar << ValueDiff.Value;

	return Ar;
}

///////////////////////////////////////////////////////////////////////////////

struct FVoxelMaterialDiff
{
	uint64 Id;
	uint32 Index;
	FVoxelMaterial Material;

	FVoxelMaterialDiff() = default;
	FVoxelMaterialDiff(uint64 Id, uint32 Index, FVoxelMaterial Material) : Id(Id), Index(Index), Material(Material) {}
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, FVoxelMaterialDiff& MaterialDiff)
{
	Ar << MaterialDiff.Id;
	Ar << MaterialDiff.Index;
	Ar << MaterialDiff.Material;

	return Ar;
}