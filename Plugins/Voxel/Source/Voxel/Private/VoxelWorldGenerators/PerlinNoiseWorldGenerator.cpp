// Copyright 2017 Phyronnaz

#pragma once

#include "VoxelPrivatePCH.h"
#include "CoreMinimal.h"
#include "PerlinNoise.h"
#include "PerlinNoiseWorldGenerator.h"

UPerlinNoiseWorldGenerator::UPerlinNoiseWorldGenerator() : Noise()
{
}

float UPerlinNoiseWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	float Density = Z;
	float Warp = 5 * GetNoise(25, X, Y, Z);
	Density += 10 * GetNoise(10 / 4.03, Warp + X, Warp + Y, Warp + Z) * 0.25;
	Density += 10 * GetNoise(10 / 1.01, Warp + X, Warp + Y, Warp + Z) * 1.00;
	Density += 1000 * GetNoise(1000 / 4.03, X, Y, Z) * 0.25;
	Density += 1000 * GetNoise(1000 / 1.01, X, Y, Z) * 1.00;
	Density += 10000 * FMath::Max(0.5f, GetNoise(10000 / 4.03, X, Y, Z)) * 0.25;
	Density += 10000 * FMath::Max(0.5f, GetNoise(10000 / 1.01, X, Y, Z)) * 1.00;
	return Density;
}

FVoxelMaterial UPerlinNoiseWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	return FVoxelMaterial();
}

void UPerlinNoiseWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{

};

float UPerlinNoiseWorldGenerator::GetNoise(float Frequency, int X, int Y, int Z)
{
	float n = Noise.Noise(X / Frequency, Y / Frequency, Z / Frequency);
	n = n - FMath::FloorToInt(n);
	return n;
}
