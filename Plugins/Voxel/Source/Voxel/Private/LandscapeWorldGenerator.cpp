// Copyright 2017 Phyronnaz

#pragma once
#include "VoxelPrivatePCH.h"
#include "LandscapeComponent.h"
#include "LandscapeWorldGenerator.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"

ALandscapeWorldGenerator::ALandscapeWorldGenerator() : Precision(1), ValueMultiplier(1)
{
};

float ALandscapeWorldGenerator::GetDefaultValue_Implementation(FIntVector Position)
{
	check(World);

	if (LocalLandscapePosition.X <= Position.X && Position.X < LocalLandscapePositionAndWidth.X && LocalLandscapePosition.Y <= Position.Y && Position.Y < LocalLandscapePositionAndWidth.Y)
	{
		int X = Position.X - LocalLandscapePosition.X;
		int Y = Position.Y - LocalLandscapePosition.Y;

		if ((Position.Z + Precision - LocalLandscapePosition.Z) * World->GetTransform().GetScale3D().Z + World->GetActorLocation().Z < Heights[X + Width * Y])
		{
			// If voxel over us in in, we're entirely in
			return -ValueMultiplier;
		}
		else if ((Position.Z - Precision - LocalLandscapePosition.Z) * World->GetTransform().GetScale3D().Z + World->GetActorLocation().Z > Heights[X + Width * Y])
		{
			// If voxel under us in out, we're entirely out
			return ValueMultiplier;
		}
		else
		{
			float Alpha = ((Position.Z - LocalLandscapePosition.Z) * World->GetTransform().GetScale3D().Z + World->GetActorLocation().Z - Heights[X + Width * Y]) / World->GetTransform().GetScale3D().Z / Precision;

			if (Alpha < 0)
			{
				Alpha *= ValueMultiplier;
			}
			else
			{
				Alpha *= ValueMultiplier;
			}

			return Alpha;
		}
	}
	else
	{
		return 0;
	}
}

FColor ALandscapeWorldGenerator::GetDefaultColor_Implementation(FIntVector Position)
{
	check(World);

	if (LocalLandscapePosition.X <= Position.X && Position.X < LocalLandscapePositionAndWidth.X && LocalLandscapePosition.Y <= Position.Y && Position.Y < LocalLandscapePositionAndWidth.Y)
	{
		int X = Position.X - LocalLandscapePosition.X;
		int Y = Position.Y - LocalLandscapePosition.Y;

		return Weights[X + Width * Y];
	}
	else
	{
		return FColor::Green;
	}
}

void ALandscapeWorldGenerator::SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld)
{
	World = VoxelWorld;
	LocalLandscapePosition = World->GetTransform().InverseTransformPosition(LandscapePosition);
	LocalLandscapePositionAndWidth = LocalLandscapePosition + FVector::OneVector * Width;
}
