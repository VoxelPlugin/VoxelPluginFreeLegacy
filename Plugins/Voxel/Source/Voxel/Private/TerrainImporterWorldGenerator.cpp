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
	}
	else
	{
		return 0;
	}
}

FColor UTerrainImporterWorldGenerator::GetDefaultColor_Implementation(FIntVector Position)
{
	check(World);

	if (!TerrainObject)
	{
		TerrainObject = Terrain.GetDefaultObject();
	}
	if (!TerrainObject)
	{
		UE_LOG(VoxelLog, Error, TEXT("Invalid terrain"));
		return FColor::Black;
	}

	Position.X -= 2;
	Position.Y -= 2;

	if (Center.X - TerrainObject->Width / 2 <= Position.X && Position.X < Center.X + TerrainObject->Width / 2 && Center.Y - TerrainObject->Width / 2 <= Position.Y && Position.Y < Center.Y + TerrainObject->Width / 2)
	{
		int X = Position.X - Center.X + TerrainObject->Width / 2;
		int Y = Position.Y - Center.Y + TerrainObject->Width / 2;

		return TerrainObject->Weights[X + TerrainObject->Width * Y];
	}
	else
	{
		return FColor::Green;
	}
}

void UTerrainImporterWorldGenerator::SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld)
{
	World = VoxelWorld;
}
