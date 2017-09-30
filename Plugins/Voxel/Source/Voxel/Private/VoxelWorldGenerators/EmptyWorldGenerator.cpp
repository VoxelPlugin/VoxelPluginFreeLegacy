#include "VoxelPrivatePCH.h"
#include "EmptyWorldGenerator.h"

float AEmptyWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	return 1;
}

FVoxelMaterial AEmptyWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	return FVoxelMaterial();
}

void AEmptyWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{

}