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
	if (LowerLimit.X <= Position.X && Position.X < UpperLimit.X && LowerLimit.Y <= Position.Y && Position.Y < UpperLimit.Y)
	{
		int X = FMath::Clamp(FMath::RoundToInt(FMath::RoundToInt(Position.X - LowerLimit.X) * SizeX / (UpperLimit.X - LowerLimit.X)), 0, (int)SizeX - 1);
		int Y = FMath::Clamp(FMath::RoundToInt(FMath::RoundToInt(Position.Y - LowerLimit.Y) * SizeY / (UpperLimit.Y - LowerLimit.Y)), 0, (int)SizeY - 1);

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

	for (int X = 0; X < SizeX; X++)
	{
		for (int Y = 0; Y < SizeY; Y++)
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
