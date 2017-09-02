// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "SphereWorldGenerator.h"

float USphereWorldGenerator::GetDefaultValue_Implementation(FIntVector Position)
{
	float Distance = static_cast<FVector>(Position).Size();
	float Alpha = FMath::Clamp(Distance - LocalRadius, -2.f, 2.f) / 2;

	if (Alpha < 0)
	{
		Alpha *= -MinValue;
	}
	else
	{
		Alpha *= MaxValue;
	}

	return Alpha * (InverseOutsideInside ? -1 : 1);
}

FColor USphereWorldGenerator::GetDefaultColor_Implementation(FIntVector Position)
{
	return DefaultColor;
}

void USphereWorldGenerator::SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld)
{
	LocalRadius = Radius / VoxelWorld->GetTransform().GetScale3D().X;
}
