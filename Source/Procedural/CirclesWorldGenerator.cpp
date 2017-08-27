// Fill out your copyright notice in the Description page of Project Settings.

#include "CirclesWorldGenerator.h"

float UCirclesWorldGenerator::GetDefaultValue_Implementation(FIntVector Position)
{
	return (FMath::Sin(Position.X / Scale.X) + FMath::Cos(Position.Y / Scale.Y)) + Position.Z > 0 ? MaxValue : MinValue;
}

FColor UCirclesWorldGenerator::GetDefaultColor_Implementation(FIntVector Position)
{
	if (Position.Z >= 1)
	{
		return BottomColor;
	}
	else if (Position.Z >= 0)
	{
		return TopColor;
	}
	else if (Position.Z >= -1)
	{
		return MiddleColor;
	}
	else
	{
		return BottomColor;
	}
}
