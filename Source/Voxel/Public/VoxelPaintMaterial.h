// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelGlobals.h"
#include "VoxelPaintMaterial.generated.h"

UENUM()
enum class EVoxelPaintMaterialType
{
	RGB,
	Index,
	Grass,
	Actor
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
	EVoxelPaintMaterialType Type = EVoxelPaintMaterialType::RGB;

	UPROPERTY()
	FLinearColor Color = FLinearColor::White;

	UPROPERTY()
	float Amount = 0;

	UPROPERTY()
	uint8 Index = 0;

	void ApplyToMaterial(FVoxelMaterial& Material, float AmountMultiplier = 1) const
	{
		switch (Type)
		{
		case EVoxelPaintMaterialType::RGB:
			Material.AddColor(Color, Amount * AmountMultiplier);
			break;
		case EVoxelPaintMaterialType::Index:
			Material.SetIndex(Index);
			break;
		case EVoxelPaintMaterialType::Grass:
			Material.SetVoxelGrassId(Index);
			break;
		case EVoxelPaintMaterialType::Actor:
			Material.SetVoxelActorId(Index);
			break;
		default:
			break;
		}
	}
};

