// Copyright 2017 Phyronnaz

#pragma once
#include "VoxelPrivatePCH.h"
#include "LandscapeComponent.h"
#include "TerrainImporterWorldGenerator.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"

float UTerrainImporterWorldGenerator::GetDefaultValue_Implementation(FIntVector Position)
{
	if (!bCreated)
	{
		Create();
	}
	if (Lower.X <= Position.X && Position.X < Upper.X && Lower.Y <= Position.Y && Position.Y < Upper.Y)
	{
		int X = FMath::Clamp(FMath::RoundToInt(FMath::RoundToInt(Position.X - Lower.X) * SizeX / (Upper.X - Lower.X)), 0, (int)SizeX - 1);
		int Y = FMath::Clamp(FMath::RoundToInt(FMath::RoundToInt(Position.Y - Lower.Y) * SizeY / (Upper.Y - Lower.Y)), 0, (int)SizeY - 1);
		return Values[X + SizeX * Y] + Position.Z > 0 ? OutValue : InValue;
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

void UTerrainImporterWorldGenerator::Create()
{

	FTexture2DMipMap* MipMap0 = &Heightmap->PlatformData->Mips[0];
	FByteBulkData* Raw = &MipMap0->BulkData;
	FColor* MipData = static_cast<FColor*>(Raw->Lock(LOCK_READ_ONLY));

	SizeX = MipMap0->SizeX;
	SizeY = MipMap0->SizeY;

	Values.SetNumUninitialized(SizeX * SizeY);

	for (uint32 X = 0; X < SizeX; X++)
	{
		for (uint32 Y = 0; Y < SizeY; Y++)
		{
			FColor PixelColor = MipData[X + SizeX * Y];

			float Value = FMath::Lerp(Top, Bottom, (PixelColor.R + PixelColor.G + PixelColor.B) / 3.f / 256.f);
			//UE_LOG(VoxelLog, Log, TEXT("%d, %d, %d"), PixelColor.R, PixelColor.G, PixelColor.B);
			Values[X + SizeX * Y] = Value;
		}
	}

	Raw->Unlock();

	bCreated = true;
}
