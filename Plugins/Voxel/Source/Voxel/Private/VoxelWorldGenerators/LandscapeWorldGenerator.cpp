#include "VoxelPrivatePCH.h"
#include "LandscapeWorldGenerator.h"

float ULandscapeWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	return 1;
}

FVoxelMaterial ULandscapeWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	return FVoxelMaterial();
}

void ULandscapeWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{

}