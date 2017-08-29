// Copyright 2017 Phyronnaz

#pragma once
#include "VoxelPrivatePCH.h"
#include "LandscapeComponent.h"
#include "TerrainImporterWorldGenerator.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "DrawDebugHelpers.h"

float UTerrainImporterWorldGenerator::GetDefaultValue_Implementation(FIntVector Position)
{
	check(World);

	if (!TerrainObject)
	{
		TerrainObject = Terrain.GetDefaultObject();
	}
	if (!TerrainObject)
	{
		UE_LOG(VoxelLog, Error, TEXT("Invalid terrain"));
		return 0;
	}

	Position.X -= 4;
	Position.Y -= 4;

	if (Center.X - TerrainObject->Width / 2 <= Position.X && Position.X < Center.X + TerrainObject->Width / 2 && Center.Y - TerrainObject->Width / 2 <= Position.Y && Position.Y < Center.Y + TerrainObject->Width / 2)
	{
		int X = Position.X - Center.X + TerrainObject->Width / 2;
		int Y = Position.Y - Center.Y + TerrainObject->Width / 2;

		return Position.Z * World->GetTransform().GetScale3D().Z - TerrainObject->Heights[X + TerrainObject->Width * Y];
		/*

		float Value = 0;

		if (Blur == EBlur::BE_GaussianBlur)
		{

			if (0 <= Y - 1)
			{
				if (0 <= X - 1)
				{
					Value += Values[X - 1 + SizeX * (Y - 1)];
				}
				Value += 2 * Values[X + SizeX * (Y - 1)];
				if (X + 1 < SizeX)
				{
					Value += Values[X + 1 + SizeX * (Y - 1)];
				}
			}

			if (0 <= X - 1)
			{
				Value += 2 * Values[X - 1 + SizeX * Y];
			}
			Value += 4 * Values[X + SizeX * Y];
			if (X + 1 < SizeX)
			{
				Value += 2 * Values[X + 1 + SizeX * Y];
			}

			if (Y + 1 < SizeY)
			{
				if (0 <= X - 1)
				{
					Value += Values[X - 1 + SizeX * (Y + 1)];
				}
				Value += 2 * Values[X + SizeX * (Y + 1)];
				if (X + 1 < SizeX)
				{
					Value += Values[X + 1 + SizeX * (Y + 1)];
				}
			}

			Value /= 16;
		}
		else if (Blur == EBlur::BE_NormalBLur)
		{

			if (0 <= Y - 1)
			{
				if (0 <= X - 1)
				{
					Value += Values[X - 1 + SizeX * (Y - 1)];
				}
				Value += Values[X + SizeX * (Y - 1)];
				if (X + 1 < SizeX)
				{
					Value += Values[X + 1 + SizeX * (Y - 1)];
				}
			}

			if (0 <= X - 1)
			{
				Value += Values[X - 1 + SizeX * Y];
			}
			Value += Values[X + SizeX * Y];
			if (X + 1 < SizeX)
			{
				Value += Values[X + 1 + SizeX * Y];
			}

			if (Y + 1 < SizeY)
			{
				if (0 <= X - 1)
				{
					Value += Values[X - 1 + SizeX * (Y + 1)];
				}
				Value += Values[X + SizeX * (Y + 1)];
				if (X + 1 < SizeX)
				{
					Value += Values[X + 1 + SizeX * (Y + 1)];
				}
			}

			Value /= 9;
		}
		else
		{
			Value = Values[X + SizeX * Y];
		}

		float Alpha = (FMath::Clamp(Value + Position.Z, -1.f, 1.F) + 1) / 2;
		return FMath::Lerp(InValue, OutValue, Alpha);
		*/
	}
	else
	{
		return 0;
	}
}

FColor UTerrainImporterWorldGenerator::GetDefaultColor_Implementation(FIntVector Position)
{
	return FColor::Green;
}

void UTerrainImporterWorldGenerator::SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld)
{
	World = VoxelWorld;
}
