// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "FlatWorldGenerator.h"
#include "VoxelMaterial.h"

UFlatWorldGenerator::UFlatWorldGenerator()
	: Height(0)
	, HardnessMultiplier(1)
	, FadeHeight(4)
{

}

float UFlatWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	return (Z >= Height) ? HardnessMultiplier : -HardnessMultiplier;
}

FVoxelMaterial UFlatWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	for (int i = 0; i < TerrainLayers.Num() - 1; i++)
	{
		if (TerrainLayers[i].Start <= Z && Z < TerrainLayers[i + 1].Start)
		{
			const uint8 Alpha = FMath::Clamp<uint8>(255 * (TerrainLayers[i + 1].Start -1 - Z) / FadeHeight, 0, 255);
			return FVoxelMaterial(TerrainLayers[i + 1].Material, TerrainLayers[i].Material, Alpha);
		}
	}
	return FVoxelMaterial(DefaultMaterial, DefaultMaterial, 0);
}

void UFlatWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{
	TerrainLayers.Sort();
}
