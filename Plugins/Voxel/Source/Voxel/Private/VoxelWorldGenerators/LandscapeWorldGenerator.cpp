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

float ALandscapeWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	check(World);

	if (LocalLandscapePosition.X <= X && X < LocalLandscapePositionAndWidth.X && LocalLandscapePosition.Y <= Y && Y < LocalLandscapePositionAndWidth.Y)
	{
		int LocalX = X - LocalLandscapePosition.X;
		int LocalY = Y - LocalLandscapePosition.Y;

		if ((Z + Precision - LocalLandscapePosition.Z) * World->GetTransform().GetScale3D().Z + World->GetActorLocation().Z < Heights[LocalX + Width * LocalY])
		{
			// If voxel over us in in, we're entirely in
			return -ValueMultiplier;
		}
		else if ((Z - Precision - LocalLandscapePosition.Z) * World->GetTransform().GetScale3D().Z + World->GetActorLocation().Z > Heights[LocalX + Width * LocalY])
		{
			// If voxel under us in out, we're entirely out
			return ValueMultiplier;
		}
		else
		{
			float Alpha = ((Z - LocalLandscapePosition.Z) * World->GetTransform().GetScale3D().Z + World->GetActorLocation().Z - Heights[LocalX + Width * LocalY]) / World->GetTransform().GetScale3D().Z / Precision;

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

FVoxelMaterial ALandscapeWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	check(World);

	if (LocalLandscapePosition.X <= X && X < LocalLandscapePositionAndWidth.X && LocalLandscapePosition.Y <= Y && Y < LocalLandscapePositionAndWidth.Y)
	{
		int LocalX = X - LocalLandscapePosition.X;
		int LocalY = Y - LocalLandscapePosition.Y;

		// TODO: FColor->FVoxelMaterial
		return FVoxelMaterial(Weights[LocalX + Width * LocalY]);
	}
	else
	{
		return FVoxelMaterial();
	}
}

void ALandscapeWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{
	World = VoxelWorld;
	LocalLandscapePosition = World->GetTransform().InverseTransformPosition(LandscapePosition);
	LocalLandscapePositionAndWidth = LocalLandscapePosition + FVector::OneVector * Width;
}
