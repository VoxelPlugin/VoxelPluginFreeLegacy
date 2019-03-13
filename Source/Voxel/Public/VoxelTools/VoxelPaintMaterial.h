// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelGlobals.h"
#include "VoxelPaintMaterial.generated.h"

UENUM()
enum class EVoxelPaintMaterialType
{
	RGBA,
	Index,
	DoubleIndexSet,
	DoubleIndexBlend,
	Grass,
	Actor
};

USTRUCT()
struct FVoxelPaintMaterialColor
{
	GENERATED_BODY()

	UPROPERTY()
	FLinearColor Color = FLinearColor::White;
	
	UPROPERTY()
	bool bPaintR = false;
	UPROPERTY()
	bool bPaintG = false;
	UPROPERTY()
	bool bPaintB = false;
	UPROPERTY()
	bool bPaintA = false;
};

USTRUCT()
struct FVoxelPaintMaterialDoubleIndex
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 IndexA = 0;
	UPROPERTY()
	uint8 IndexB = 0;
	UPROPERTY()
	float Blend = 0;

	UPROPERTY()
	bool bSetIndexA = false;
	UPROPERTY()
	bool bSetIndexB = false;
	UPROPERTY()
	bool bSetBlend = false;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelPaintMaterial
{
	GENERATED_BODY()

	FVoxelPaintMaterial() = default;
	FVoxelPaintMaterial(EVoxelPaintMaterialType Type)
		: Type(Type)
	{
	}

	UPROPERTY()
	EVoxelPaintMaterialType Type = EVoxelPaintMaterialType::RGBA;
	
	UPROPERTY()
	float Amount = 0;

	UPROPERTY()
	FVoxelPaintMaterialColor Color;

	UPROPERTY()
	uint8 Index = 0;

	UPROPERTY()
	FVoxelPaintMaterialDoubleIndex DoubleIndex;


	void ApplyToMaterial(FVoxelMaterial& Material, float AmountMultiplier = 1) const
	{
		switch (Type)
		{
		case EVoxelPaintMaterialType::RGBA:
		{
			Material.AddColor(Color.Color, Amount * AmountMultiplier, Color.bPaintR, Color.bPaintG, Color.bPaintB, Color.bPaintA);
			break;
		}
		case EVoxelPaintMaterialType::Index:
		{
			Material.SetIndex(Index);
			break;
		}
		case EVoxelPaintMaterialType::DoubleIndexSet:
		{
			if (DoubleIndex.bSetIndexA)
			{
				Material.SetIndexA(DoubleIndex.IndexA);
			}
			if (DoubleIndex.bSetIndexB)
			{
				Material.SetIndexB(DoubleIndex.IndexB);
			}
			if (DoubleIndex.bSetBlend)
			{
				Material.SetBlend(FMath::Clamp(FMath::RoundToInt(DoubleIndex.Blend * 255.999f), 0, 255));
			}
			break;
		}
		case EVoxelPaintMaterialType::DoubleIndexBlend:
		{
			if (Index != Material.GetIndexA() && Index != Material.GetIndexB())
			{
				if (Material.GetBlend() < 128)
				{
					Material.SetIndexB(Index);
					Material.SetBlend(0);
				}
				else
				{
					Material.SetIndexA(Index);
					Material.SetBlend(255);
				}
			}
			if (Index == Material.GetIndexA())
			{
				Material.SetBlend(FMath::Clamp(FMath::RoundToInt(FMath::Lerp<int>(Material.GetBlend(), 0, Amount)), 0, 255));
			}
			else if (Index == Material.GetIndexB())
			{
				Material.SetBlend(FMath::Clamp(FMath::RoundToInt(FMath::Lerp<int>(Material.GetBlend(), 255, Amount)), 0, 255));
			}
		}
		case EVoxelPaintMaterialType::Grass:
		{
			Material.SetVoxelGrassId(Index);
			break;
		}
		case EVoxelPaintMaterialType::Actor:
		{
			Material.SetVoxelActorId(Index);
			break;
		}
		default:
			check(false);
		}
	}
};

