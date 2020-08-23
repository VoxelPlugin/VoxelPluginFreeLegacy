// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelAxisDependencies.generated.h"

enum class EVoxelAxisDependencies : uint8
{
	Constant,
	X,
	XY,
	XYZ,
};

namespace EVoxelAxisDependenciesFlags
{
	enum : uint8
	{
		X   = 0x1,
		Y   = 0x2,
		Z   = 0x4,
		XY  = X | Y,
		XZ  = X | Z,
		YZ  = Y | Z,
		XYZ = X | Y | Z
	};
}

UENUM()
enum class EVoxelFunctionAxisDependencies : uint8
{
	X,
	// X was run
	XYWithCache,
	// X wasn't run
	XYWithoutCache,
	// XY was run
	XYZWithCache,
	// XY wasn't run
	XYZWithoutCache
};

namespace FVoxelAxisDependencies
{
	inline EVoxelAxisDependencies GetVoxelAxisDependenciesFromFlag(uint8 Flag)
	{
		if (Flag & EVoxelAxisDependenciesFlags::Z)
		{
			return EVoxelAxisDependencies::XYZ;
		}
		else if (Flag & EVoxelAxisDependenciesFlags::Y)
		{
			return EVoxelAxisDependencies::XY;
		}
		else if (Flag & EVoxelAxisDependenciesFlags::X)
		{
			return EVoxelAxisDependencies::X;
		}
		else
		{
			return EVoxelAxisDependencies::Constant;
		}
	}

	inline bool IsConstant(uint8 Flag)
	{
		return GetVoxelAxisDependenciesFromFlag(Flag) == EVoxelAxisDependencies::Constant;
	}

	inline TArray<EVoxelFunctionAxisDependencies, TFixedAllocator<5>> GetAllFunctionDependencies()
	{
		return
		{
			EVoxelFunctionAxisDependencies::X,
			EVoxelFunctionAxisDependencies::XYWithCache,
			EVoxelFunctionAxisDependencies::XYWithoutCache,
			EVoxelFunctionAxisDependencies::XYZWithCache,
			EVoxelFunctionAxisDependencies::XYZWithoutCache
		};
	}

	VOXELGRAPH_API FString ToString(EVoxelAxisDependencies Dependencies);
	VOXELGRAPH_API FString ToString(EVoxelFunctionAxisDependencies Dependencies);
}