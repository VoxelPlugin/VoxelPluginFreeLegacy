// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "FlatWorldGenerator.h"
#include "VoxelMaterial.h"

UFlatWorldGenerator::UFlatWorldGenerator() : Height(0), ValueMultiplier(1)
{

}

float UFlatWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	return (Z >= Height) ? ValueMultiplier : -ValueMultiplier;
}

FVoxelMaterial UFlatWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	for (auto Layer : TerrainLayers)
	{
		if (Layer.Start <= Z && Z < Layer.Start + Layer.Height)
		{
			// if we are in this layer
			return FVoxelMaterial(Layer.Material1, Layer.Material2, Layer.Alpha);
		}
	}
	return FVoxelMaterial(DefaultMaterial1, DefaultMaterial2, DefaultAlpha);
}

void UFlatWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{

}
