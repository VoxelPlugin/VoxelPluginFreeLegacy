// Copyright 2017 Phyronnaz

#pragma once

#include "VoxelPrivatePCH.h"
#include "CoreMinimal.h"
#include "PerlinNoise.h"
#include "PerlinNoiseWorldGenerator.h"

UPerlinNoiseWorldGenerator::UPerlinNoiseWorldGenerator() : Noise()
{
}

float UPerlinNoiseWorldGenerator::GetDefaultValue_Implementation(FIntVector Position)
{
	float n = Noise.Noise(Position.X / 10.f, Position.Y / 10.f, Position.Z / 10.f);
	n = n - FMath::FloorToInt(n);

	return 2 * n - 1;
}

FColor UPerlinNoiseWorldGenerator::GetDefaultColor_Implementation(FIntVector Position)
{
	return FColor::Green;
}

void UPerlinNoiseWorldGenerator::SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld)
{

};
