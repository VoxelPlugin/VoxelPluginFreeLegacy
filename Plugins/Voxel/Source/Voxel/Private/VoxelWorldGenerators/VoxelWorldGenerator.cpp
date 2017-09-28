// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelWorldGenerator.h"

float AVoxelWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	return 0;
}

FColor AVoxelWorldGenerator::GetDefaultColor(int X, int Y, int Z)
{
	return FColor::Black;
}

void AVoxelWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{

}