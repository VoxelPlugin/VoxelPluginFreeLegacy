// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "SphereWorldGenerator.h"

float USphereWorldGenerator::GetDefaultValue_Implementation(FIntVector Position)
{
	float Distance = static_cast<FVector>(Position).Size();
	float Alpha = FMath::Clamp(Distance - Radius, -2.f, 2.f) / 2;

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
	float Distance = static_cast<FVector>(Position).Size();
	float Alpha = (Radius * 0.9f - Distance) / Radius * 0.9f;

	return FLinearColor::LerpUsingHSV(FColor::Red, FColor::Green, FMath::Clamp(Alpha, 0.f, 1.f)).ToFColor(true);
}
