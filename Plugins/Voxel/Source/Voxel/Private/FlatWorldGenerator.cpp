// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "FlatWorldGenerator.h"

AFlatWorldGenerator::AFlatWorldGenerator() : Height(0), DefaultColor(FColor::White), ValueMultiplier(1)
{

}

float AFlatWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	return (Z > Height) ? ValueMultiplier : -ValueMultiplier;
}

FColor AFlatWorldGenerator::GetDefaultColor(int X, int Y, int Z)
{
	for (auto Layer : TerrainLayers)
	{
		if (Layer.Start <= Z && Z < Layer.Start + Layer.Height)
		{
			return Layer.Color.ToFColor(true);
		}
	}
	return DefaultColor.ToFColor(true);
}

void AFlatWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{

}
