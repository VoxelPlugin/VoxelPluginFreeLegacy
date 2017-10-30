// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "BufferArchive.h"
#include "MemoryReader.h"
#include "VoxelBox.h"
#include "VoxelAsset.generated.h"

enum VOXEL_API EVoxelValueType : uint8
{
	// Value should be used
	UseValue,

	// Use if A * B >= 0
	UseValueIfSameSign,

	// Use if A * B <= 0
	UseValueIfDifferentSign,

	// Value is not valid
	IgnoreValue
};

enum VOXEL_API EVoxelMaterialType : uint8
{
	UseMaterial,
	IgnoreMaterial
};

struct VOXEL_API FVoxelType
{
	const uint8 Value;

	FVoxelType(uint8 Value)
		: Value(Value)
	{
	}

	FVoxelType(EVoxelValueType ValueType, EVoxelMaterialType MaterialType)
		: Value(ValueType | (MaterialType << 4))
	{
	}

	EVoxelValueType GetValueType() const
	{
		return (EVoxelValueType)(0x0F & Value);
	}

	EVoxelMaterialType GetMaterialType() const
	{
		return (EVoxelMaterialType)(Value >> 4);
	}
};

struct VOXEL_API FDecompressedVoxelAsset
{
	virtual ~FDecompressedVoxelAsset() {};

	virtual float GetValue(const int X, const int Y, const int Z) = 0;
	virtual FVoxelMaterial GetMaterial(const int X, const int Y, const int Z) = 0;
	virtual FVoxelType GetVoxelType(const int X, const int Y, const int Z) = 0;
	virtual FVoxelBox GetBounds() = 0;
};

/**
*
*/
UCLASS()
class VOXEL_API UVoxelAsset : public UObject
{
	GENERATED_BODY()

public:
	UVoxelAsset(const FObjectInitializer& ObjectInitializer);
	virtual ~UVoxelAsset();

	virtual void InitFromAsset(FDecompressedVoxelAsset* Asset);

	/**
	 * When inheriting, must be override to create Asset before calling Super
	 * @return	Asset
	 * @return	Success?
	 */
	virtual bool GetDecompressedAsset(FDecompressedVoxelAsset*& Asset, const float VoxelSize);

protected:
	UPROPERTY()
		TArray<uint8> Data;

	virtual void AddAssetToArchive(FBufferArchive& ToBinary, FDecompressedVoxelAsset* Asset);
	virtual void GetAssetFromArchive(FMemoryReader& FromBinary, FDecompressedVoxelAsset* Asset);
};