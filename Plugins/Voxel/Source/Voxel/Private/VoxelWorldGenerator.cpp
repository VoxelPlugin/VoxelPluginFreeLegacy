// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelWorldGenerator.h"

int UVoxelWorldGenerator::GetDefaultValue(FIntVector Position)
{
	return (Position.Z == 0) ? 0 : ((Position.Z > 0) ? 100 : -100);
}

FColor UVoxelWorldGenerator::GetDefaultColor(FIntVector Position)
{
	return (Position.Z == 0) ? FColor::White : ((Position.Z > 0) ? FColor::Red : FColor::Green);
}