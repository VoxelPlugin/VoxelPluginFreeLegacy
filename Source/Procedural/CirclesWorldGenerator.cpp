// Fill out your copyright notice in the Description page of Project Settings.

#include "CirclesWorldGenerator.h"

ACirclesWorldGenerator::ACirclesWorldGenerator() : Scale(10, 10), TopColor(230, 255, 0, 255), MiddleColor(255, 0, 26, 255), BottomColor(13, 0, 255, 255), MaxValue(1), MinValue(-1)
{

}

float ACirclesWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	return (FMath::Sin(X / Scale.X) + FMath::Cos(Y / Scale.Y)) + Z > 0 ? MaxValue : MinValue;
}

FColor ACirclesWorldGenerator::GetDefaultColor(int X, int Y, int Z)
{
	if (Z >= 1)
	{
		return BottomColor;
	}
	else if (Z >= 0)
	{
		return TopColor;
	}
	else if (Z >= -1)
	{
		return MiddleColor;
	}
	else
	{
		return BottomColor;
	}
}

void ACirclesWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{

}

