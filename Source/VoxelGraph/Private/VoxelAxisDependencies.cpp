// Copyright 2020 Phyronnaz

#include "VoxelAxisDependencies.h"

FString FVoxelAxisDependencies::ToString(EVoxelAxisDependencies Dependencies)
{
	switch (Dependencies)
	{
	case EVoxelAxisDependencies::Constant:
		return "Constant";
	case EVoxelAxisDependencies::X:
		return "X";
	case EVoxelAxisDependencies::XY:
		return "XY";
	case EVoxelAxisDependencies::XYZ:
		return "XYZ";
	default:
		check(false);
		return "";
	}
}

FString FVoxelAxisDependencies::ToString(EVoxelFunctionAxisDependencies Dependencies)
{
	switch (Dependencies)
	{
	case EVoxelFunctionAxisDependencies::X:
		return "X";
	case EVoxelFunctionAxisDependencies::XYWithCache:
		return "XYWithCache";
	case EVoxelFunctionAxisDependencies::XYWithoutCache:
		return "XYWithoutCache";
	case EVoxelFunctionAxisDependencies::XYZWithCache:
		return "XYZWithCache";
	case EVoxelFunctionAxisDependencies::XYZWithoutCache:
		return "XYZWithoutCache";
	default:
		check(false);
		return "";
	}
}