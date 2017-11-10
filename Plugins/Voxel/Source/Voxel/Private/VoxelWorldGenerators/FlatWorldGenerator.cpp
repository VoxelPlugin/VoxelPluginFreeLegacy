// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "FlatWorldGenerator.h"
#include "VoxelMaterial.h"

UFlatWorldGenerator::UFlatWorldGenerator()
	: TerrainHeight(0)
	, HardnessMultiplier(1)
	, FadeHeight(4)
{

}

float UFlatWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	return (Z >= TerrainHeight) ? HardnessMultiplier : -HardnessMultiplier;
}

FVoxelMaterial UFlatWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	if (Z < TerrainLayers[0].Start)
	{
		return FVoxelMaterial(TerrainLayers[0].Material, TerrainLayers[0].Material, 255);
	}
	const int LastIndex = TerrainLayers.Num() - 1;
	if (Z >= TerrainLayers[LastIndex].Start)
	{
		return FVoxelMaterial(TerrainLayers[LastIndex].Material, TerrainLayers[LastIndex].Material, (LastIndex % 2 == 0) ? 255 : 0);
	}
	for (int i = 0; i < TerrainLayers.Num() - 1; i++)
	{
		if (TerrainLayers[i].Start <= Z && Z < TerrainLayers[i + 1].Start)
		{
			const uint8 Alpha = FMath::Clamp<int>(255 * (TerrainLayers[i + 1].Start - 1 - Z) / FadeHeight, 0, 255);

			// Alternate first material to avoid problem with alpha smoothing
			if (i % 2 == 0)
			{
				return FVoxelMaterial(TerrainLayers[i + 1].Material, TerrainLayers[i].Material, Alpha);
			}
			else
			{
				return FVoxelMaterial(TerrainLayers[i].Material, TerrainLayers[i + 1].Material, 255 - Alpha);
			}
		}
	}
	return FVoxelMaterial();
}

void UFlatWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{
	TerrainLayers.Sort([](const FFlatWorldLayer& Left, const FFlatWorldLayer& Right) { return Left.Start < Right.Start; });
}
