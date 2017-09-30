// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelWorldGenerator.h"

float AVoxelWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	return 0;
}

FVoxelMaterial AVoxelWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	return FVoxelMaterial();
}

void AVoxelWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{

}