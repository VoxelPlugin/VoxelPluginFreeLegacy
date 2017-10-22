// Fill out your copyright notice in the Description page of Project Settings.

#include "FunnyWorldGenerator.h"

UFunnyWorldGenerator::UFunnyWorldGenerator()
	: ValueMultiplier(1)
	, SphereLayerHeight(100)
	, Radius(10)
	, RadiusDivisor(1.5)
{

}

float UFunnyWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	if (Z < 0)
	{
		return -ValueMultiplier;
	}
	else if (SphereLayerHeight <= Z && Z <= SphereLayerHeight + 2 * Radius)
	{
		int NX = FMath::Abs(X) % (2 * Radius);
		int NY = FMath::Abs(Y) % (2 * Radius);
		int NZ = Z - SphereLayerHeight;
		NX -= Radius;
		NY -= Radius;
		NZ -= Radius;

		return FVector(NX, NY, NZ).Size() - Radius / RadiusDivisor;
	}
	else
	{
		return ValueMultiplier;
	}
}

FVoxelMaterial UFunnyWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	if (Z >= 1)
	{
		return BottomMaterial;
	}
	else if (Z >= 0)
	{
		return TopMaterial;
	}
	else if (Z >= -1)
	{
		return MiddleMaterial;
	}
	else
	{
		return BottomMaterial;
	}
}

void UFunnyWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{

}

