// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "SphereWorldGenerator.h"

ASphereWorldGenerator::ASphereWorldGenerator() : Radius(1000), InverseOutsideInside(false), ValueMultiplier(1)
{

}

float ASphereWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	// Distance to the center
	float Distance = FVector(X, Y, Z).Size();

	// Alpha = -1 inside the sphere, 1 outside and an interpolated value on intersection
	float Alpha = FMath::Clamp(Distance - LocalRadius, -2.f, 2.f) / 2;

	// Multiply by custom ValueMultiplier (allows for rocks harder to mine)
	Alpha *= ValueMultiplier;

	return Alpha * (InverseOutsideInside ? -1 : 1);
}

FVoxelMaterial ASphereWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	return DefaultMaterial;
}

void ASphereWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{
	LocalRadius = Radius / VoxelWorld->GetVoxelSize();
}

FVector ASphereWorldGenerator::GetUpVector(int X, int Y, int Z)
{
	return FVector(X, Y, Z).GetSafeNormal();
}
