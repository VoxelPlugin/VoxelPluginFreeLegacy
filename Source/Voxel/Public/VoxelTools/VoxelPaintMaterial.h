// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelPaintMaterial.generated.h"

UENUM()
enum class EVoxelPaintMaterialType
{
	RGB,
	SingleIndex,
	DoubleIndexSet,
	DoubleIndexBlend
};

USTRUCT()
struct FVoxelPaintMaterialColor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FLinearColor Color = FLinearColor::White;
	
	UPROPERTY(EditAnywhere)
	bool bPaintR = true;
	UPROPERTY(EditAnywhere)
	bool bPaintG = true;
	UPROPERTY(EditAnywhere)
	bool bPaintB = true;
	UPROPERTY(EditAnywhere)
	bool bPaintA = true;
};

USTRUCT()
struct FVoxelPaintMaterialDoubleIndexSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	uint8 IndexA = 0;
	UPROPERTY(EditAnywhere)
	uint8 IndexB = 0;
	UPROPERTY(EditAnywhere)
	float Blend = 0;

	UPROPERTY(EditAnywhere)
	bool bSetIndexA = true;
	UPROPERTY(EditAnywhere)
	bool bSetIndexB = true;
	UPROPERTY(EditAnywhere)
	bool bSetBlend = true;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelPaintMaterial
{
	GENERATED_BODY()

public:
	FVoxelPaintMaterial() = default;

	static FVoxelPaintMaterial CreateRGB(FLinearColor Color, float Amount, bool bPaintR, bool bPaintG, bool bPaintB, bool bPaintA);
	static FVoxelPaintMaterial CreateSingleIndex(uint8 Index);
	static FVoxelPaintMaterial CreateDoubleIndexSet(uint8 IndexA, uint8 IndexB, float Blend, bool bSetIndexA, bool bSetIndexB, bool bSetBlend);
	static FVoxelPaintMaterial CreateDoubleIndexBlend(uint8 Index, float Amount);

	void ApplyToMaterial(FVoxelMaterial& Material, float AmountMultiplier = 1) const;

private:
	UPROPERTY(EditAnywhere)
	EVoxelPaintMaterialType Type = EVoxelPaintMaterialType::RGB;
	
	UPROPERTY(EditAnywhere, meta = (UIMin = 0, UIMax = 1))
	float Amount = 0.5;

	UPROPERTY(EditAnywhere)
	FVoxelPaintMaterialColor Color;

	UPROPERTY(EditAnywhere)
	uint8 Index = 0;

	UPROPERTY(EditAnywhere)
	FVoxelPaintMaterialDoubleIndexSet DoubleIndexSet;

	friend class FVoxelPaintMaterialCustomization;
};

