// Copyright 2017 Phyronnaz

#pragma once

#include "VoxelPrivatePCH.h"
#include "CoreMinimal.h"
#include "PerlinNoiseWorldGenerator.h"

UPerlinNoiseWorldGenerator::UPerlinNoiseWorldGenerator() : Noise()
{
}

float UPerlinNoiseWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	float Density = Z;

	float x = X;
	float y = Y;
	float z = Z;

	//Density += 1000 * Noise.GetSimplexFractal(x / 100, y / 100, z / 100);

	Noise.GradientPerturb(x, y, z);

	Density += 5 * Noise.GetSimplexFractal(x, y, z);

	return Density;
}

FVoxelMaterial UPerlinNoiseWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	if (Z < -10)
	{
		return FVoxelMaterial(0, 0, 0);
	}
	else if (Z < 0)
	{
		return FVoxelMaterial(0, 1, FMath::Clamp<uint8>((10 + Z) * 256.f / 10.f, 0, 255));
	}
	else if (Z < 10)
	{
		return FVoxelMaterial(2, 1, 255 - FMath::Clamp<uint8>(Z * 256.f / 10.f, 0, 255));
	}
	else if (Z < 100)
	{
		return FVoxelMaterial(2, 2, 0);
	}
	else
	{
		return FVoxelMaterial(3, 3, 255);
	}
}

void UPerlinNoiseWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{
	Noise = FastNoise();
	Noise.SetGradientPerturbAmp(45);
	Noise.SetFrequency(0.02);
};