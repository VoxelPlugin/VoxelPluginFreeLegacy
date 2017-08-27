// Copyright 2017 Phyronnaz

#pragma once
#include "VoxelPrivatePCH.h"
#include "LandscapeComponent.h"
#include "TerrainImporterWorldGenerator.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"

float ATerrainImporterWorldGenerator::GetDefaultValue_Implementation(FIntVector Position)
{
	if (!bCreated)
	{
		Create();
	}
	if (Lower.X <= Position.X && Position.X < Upper.X && Lower.Y <= Position.Y && Position.Y < Upper.Y)
	{
		int X = FMath::Clamp(FMath::RoundToInt(FMath::RoundToInt(Position.X - Lower.X) * SizeX / (Upper.X - Lower.X)), 0, (int)SizeX - 1);
		int Y = FMath::Clamp(FMath::RoundToInt(FMath::RoundToInt(Position.Y - Lower.Y) * SizeY / (Upper.Y - Lower.Y)), 0, (int)SizeY - 1);
		return Values[X + SizeX * Y] > 0 ? OutValue : InValue;
	}
	else
	{
		return  0;
	}
}

FColor ATerrainImporterWorldGenerator::GetDefaultColor_Implementation(FIntVector Position)
{
	return FColor::Green;
}

void ATerrainImporterWorldGenerator::Create()
{
	if (Landscape == NULL)
	{
		return;
	}
	FTexture2DMipMap* MyMipMap = &Landscape->LandscapeComponents[0]->HeightmapTexture->PlatformData->Mips[0];
	FByteBulkData* RawImageData = &MyMipMap->BulkData;
	FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));

	SizeX = MyMipMap->SizeX;
	SizeY = MyMipMap->SizeY;

	for (uint32 X = 0; X < SizeX; X++)
	{
		for (uint32 Y = 0; Y < SizeY; Y++)
		{
			FColor PixelColor = FormatedImageData[Y * SizeY + X];
			float Value = PixelColor.R * (Bottom + Top) / 256 - Bottom;
			Values[X + SizeX * Y] = Value;
		}
	}
	bCreated = true;
}
