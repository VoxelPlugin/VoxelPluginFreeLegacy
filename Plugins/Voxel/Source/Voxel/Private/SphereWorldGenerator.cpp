// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "SphereWorldGenerator.h"

ASphereWorldGenerator::ASphereWorldGenerator() : Radius(10), InverseOutsideInside(false), ValueMultiplier(1)
{

}

float ASphereWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	float Distance = FVector(X, Y, Z).Size();
	float Alpha = FMath::Clamp(Distance - LocalRadius, -2.f, 2.f) / 2;

	Alpha *= ValueMultiplier;

	return Alpha * (InverseOutsideInside ? -1 : 1);
}

FColor ASphereWorldGenerator::GetDefaultColor(int X, int Y, int Z)
{
	return DefaultColor;
}

void ASphereWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{
	LocalRadius = Radius / VoxelWorld->GetTransform().GetScale3D().X;
}
