// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelConfigEnums.h"
#include "Containers/StaticArray.h"
#include "VoxelPaintMaterial.generated.h"

UENUM(BlueprintType)
enum class EVoxelPaintMaterialType : uint8
{
	RGB,
	FiveWayBlend,
	SingleIndex,
	DoubleIndexSet,
	DoubleIndexBlend,
	UVs
};

USTRUCT(BlueprintType)
struct FVoxelPaintMaterialColor
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	FLinearColor Color = FLinearColor::White;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bPaintR = true;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bPaintG = true;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bPaintB = true;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bPaintA = true;
};

USTRUCT(BlueprintType)
struct FVoxelPaintMaterialFiveWayBlend
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel", meta = (UIMin = 0, UIMax = 4, ClampMin = 0, ClampMax = 4))
	int32 Channel = 0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel", meta = (UIMin = 0, UIMax = 1, ClampMin = 0, ClampMax = 1))
	float TargetValue = 1.f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bPaintR = true;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bPaintG = true;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bPaintB = true;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bPaintA = true;
};

USTRUCT(BlueprintType)
struct FVoxelPaintMaterialDoubleIndexSet
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	uint8 IndexA = 0;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	uint8 IndexB = 0;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel", meta = (UIMin = 0, UIMax = 1))
	float Blend = 0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bSetIndexA = true;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bSetIndexB = true;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bSetBlend = true;
};

USTRUCT(BlueprintType)
struct FVoxelPaintMaterialUV
{
	GENERATED_BODY()
		
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	uint8 Channel = 0;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	FVector2D UV = FVector2D::ZeroVector;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bPaintU = true;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bPaintV = true;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelPaintMaterial
{
	GENERATED_BODY()

public:
	FVoxelPaintMaterial() = default;

	static FVoxelPaintMaterial CreateRGB(FLinearColor Color, bool bPaintR, bool bPaintG, bool bPaintB, bool bPaintA);
	static FVoxelPaintMaterial CreateFiveWayBlend(int32 Channel, float TargetValue, bool bPaintR, bool bPaintG, bool bPaintB, bool bPaintA);
	static FVoxelPaintMaterial CreateSingleIndex(uint8 Index);
	static FVoxelPaintMaterial CreateDoubleIndexSet(uint8 IndexA, uint8 IndexB, float Blend, bool bSetIndexA, bool bSetIndexB, bool bSetBlend);
	static FVoxelPaintMaterial CreateDoubleIndexBlend(uint8 Index);
	static FVoxelPaintMaterial CreateUV(uint8 Channel, FVector2D UV, bool bPaintU, bool bPaintV);

	void ApplyToMaterial(FVoxelMaterial& Material, float Strength) const;

public:
	// NOTE: Don't edit those directly, use the functions above instead!
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool bRestrictType = false;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelMaterialConfig MaterialConfigToRestrictTo = EVoxelMaterialConfig::RGB;
#endif
	
	// The type of painting
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	EVoxelPaintMaterialType Type = EVoxelPaintMaterialType::RGB;

	// Color to paint
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterialColor Color;

	// Index to paint
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	uint8 Index = 0;

	// Double index to paint
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterialDoubleIndexSet DoubleIndexSet;

	// UVs to paint
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterialUV UV;

	// For 5 way blends
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterialFiveWayBlend FiveWayBlend;
};