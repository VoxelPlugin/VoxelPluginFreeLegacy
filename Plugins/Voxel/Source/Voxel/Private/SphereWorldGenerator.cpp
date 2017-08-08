// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelPrivatePCH.h"
#include "SphereWorldGenerator.h"

int USphereWorldGenerator::GetDefaultValue(FIntVector Position)
{
	float Distance = ((FVector)Position).Size();

	float Alpha = FMath::Clamp(Radius - Distance, -1.f, 1.f);

	return (int)(127 * Alpha) * (InverseOutsideInside ? 1 : -1);
}

FColor USphereWorldGenerator::GetDefaultColor(FIntVector Position)
{
	float Distance = ((FVector)Position).Size();
	float Alpha = (Radius * 0.9f - Distance) / Radius * 0.9f;

	return FLinearColor::LerpUsingHSV(FColor::Red, FColor::Green, FMath::Clamp(Alpha, 0.f, 1.f)).ToFColor(true);
}
