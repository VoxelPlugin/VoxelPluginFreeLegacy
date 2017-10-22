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
	float n = Noise.Noise(X / 10.f, Y / 10.f, Z / 10.f);
	n = n - FMath::FloorToInt(n);

	return 2 * n - 1;
}

FVoxelMaterial UPerlinNoiseWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	return FVoxelMaterial();
}

void UPerlinNoiseWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{

};
