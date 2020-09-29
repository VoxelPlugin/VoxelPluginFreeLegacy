// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelMaterial.h"
#include "VoxelPaintMaterial.generated.h"

class UMaterialInterface;
class UVoxelMaterialCollectionBase;

USTRUCT(BlueprintType)
struct FVoxelPaintMaterial_MaterialCollectionChannel
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	uint8 Channel = 0;

	operator uint8() const{ return Channel; }
};

UENUM(BlueprintType)
enum class EVoxelPaintMaterialType : uint8
{
	Color,
	FiveWayBlend,
	SingleIndex,
	MultiIndex,
	MultiIndexWetness,
	MultiIndexRaw,
	UV
};

USTRUCT(BlueprintType)
struct FVoxelPaintMaterialColor
{
	GENERATED_BODY()

	// Set to true if you want to use the unreal color picker
	// Set to false if you want to set the bytes manually
	//
	// The unreal color picker will write linear colors to LinearColor, and sRGB colors to Color
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	bool bUseLinearColor = true;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FLinearColor LinearColor = FLinearColor::Transparent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FColor Color = FColor::Transparent;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	bool bPaintR = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	bool bPaintG = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	bool bPaintB = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	bool bPaintA = true;
};

USTRUCT(BlueprintType)
struct FVoxelPaintMaterialFiveWayBlend
{
	GENERATED_BODY()

	// Between 0 and 4. 1,2,3,4 => R,G,B,A. 0 => material displayed by default
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel", meta = (UIMin = 0, UIMax = 4, ClampMin = 0, ClampMax = 4))
	int32 Channel = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel", meta = (UIMin = 0, UIMax = 1, ClampMin = 0, ClampMax = 1))
	float TargetValue = 1.f;

	// These channels will have their strength locked, and will stay the same
	// Useful eg to paint _under_ rocks: lock the rock channel, and paint the channel you want to put under them
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	TArray<uint8> LockedChannels;

	// If true, will ignore Alpha
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AdvancedDisplay, Category = "Voxel")
	bool bFourWayBlend = false;
};

USTRUCT(BlueprintType)
struct FVoxelPaintMaterialSingleIndex
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterial_MaterialCollectionChannel Channel;
};
	
USTRUCT(BlueprintType)
struct FVoxelPaintMaterialMultiIndex
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterial_MaterialCollectionChannel Channel;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel", meta = (UIMin = 0, UIMax = 1, ClampMin = 0, ClampMax = 1))
	float TargetValue = 1.f;

	// These channels will have their strength locked, and will stay the same
	// Useful eg to paint _under_ rocks: lock the rock channel, and paint the channel you want to put under them
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	TArray<FVoxelPaintMaterial_MaterialCollectionChannel> LockedChannels;
};

USTRUCT(BlueprintType)
struct FVoxelPaintMaterialMultiIndexWetness
{
	GENERATED_BODY()
		
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel", meta = (UIMin = 0, UIMax = 1, ClampMin = 0, ClampMax = 1))
	float TargetValue = 1.f;
};

USTRUCT(BlueprintType)
struct FVoxelPaintMaterialMultiIndexRaw
{
	GENERATED_BODY()
		
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterial_MaterialCollectionChannel Channel0;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel", meta = (UIMin = 0, UIMax = 1))
	float Strength0 = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterial_MaterialCollectionChannel Channel1;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel", meta = (UIMin = 0, UIMax = 1))
	float Strength1 = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterial_MaterialCollectionChannel Channel2;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel", meta = (UIMin = 0, UIMax = 1))
	float Strength2 = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterial_MaterialCollectionChannel Channel3;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel", meta = (UIMin = 0, UIMax = 1))
	float Strength3 = 0.f;
};

USTRUCT(BlueprintType)
struct FVoxelPaintMaterialUV
{
	GENERATED_BODY()
		
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel", meta = (UIMin = 0, UIMax = 4))
	int32 Channel = 0;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVector2D UV = FVector2D::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	bool bPaintU = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	bool bPaintV = true;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelPaintMaterial
{
	GENERATED_BODY()

public:
	FVoxelPaintMaterial() = default;

	void ApplyToMaterial(FVoxelMaterial& Material, float Strength) const;

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, EditAnywhere, Category = "Voxel")
	bool bRestrictType = false;

	UPROPERTY(Transient, EditAnywhere, Category = "Voxel")
	EVoxelMaterialConfig MaterialConfigToRestrictTo = EVoxelMaterialConfig::RGB;

	UPROPERTY(Transient, EditAnywhere, Category = "Voxel")
	UVoxelMaterialCollectionBase* PreviewMaterialCollection = nullptr;
#endif

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	EVoxelPaintMaterialType Type = EVoxelPaintMaterialType::FiveWayBlend;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterialColor Color;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterialSingleIndex SingleIndex;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterialMultiIndex MultiIndex;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterialMultiIndexWetness MultiIndexWetness;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterialMultiIndexRaw MultiIndexRaw;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterialUV UV;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel")
	FVoxelPaintMaterialFiveWayBlend FiveWayBlend;
};