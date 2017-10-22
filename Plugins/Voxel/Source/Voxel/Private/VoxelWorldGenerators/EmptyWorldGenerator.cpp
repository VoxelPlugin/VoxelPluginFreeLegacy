#include "VoxelPrivatePCH.h"
#include "EmptyWorldGenerator.h"

float UEmptyWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	return 1;
}

FVoxelMaterial UEmptyWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	return FVoxelMaterial();
}

void UEmptyWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{

}