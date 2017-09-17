#include "VoxelPrivatePCH.h"
#include "EmptyWorldGenerator.h"

float AEmptyWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	return 1;
}

FColor AEmptyWorldGenerator::GetDefaultColor(int X, int Y, int Z)
{
	return FColor::Black;
}

void AEmptyWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{

}