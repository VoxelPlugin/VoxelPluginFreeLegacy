// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "BufferArchive.h"
#include "MemoryReader.h"
#include "VoxelAsset.generated.h"

enum EVoxelType : uint8
{
	// Value should be used
	UseValue,

	// Use if A * B >= 0
	UseValueIfSameSign,

	// Value is not valid
	IgnoreValue
};

struct FVoxelBox
{
	FIntVector Min;
	FIntVector Max;

	FORCEINLINE bool IsInside(const int X, const int Y, const int Z) const
	{
		return ((X > Min.X) && (X < Max.X) && (Y > Min.Y) && (Y < Max.Y) && (Z > Min.Z) && (Z < Max.Z));
	}
};

struct VOXEL_API FDecompressedVoxelAsset
{
	virtual ~FDecompressedVoxelAsset() {};

	virtual float GetValue(const int X, const int Y, const int Z) = 0;
	virtual FVoxelMaterial GetMaterial(const int X, const int Y, const int Z) = 0;
	virtual EVoxelType GetVoxelType(const int X, const int Y, const int Z) = 0;
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