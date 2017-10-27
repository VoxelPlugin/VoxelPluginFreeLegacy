// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelDataAsset.generated.h"

enum EVoxelType : uint8
{
	UseValue,
	UseValueIfSameSign,
	IgnoreValue
};

struct FDecompressedVoxelDataAsset
{
	int32 SizeX;
	int32 SizeY;
	int32 SizeZ;

	TArray<float> Values;
	TArray<FVoxelMaterial> Materials;

	TArray<uint8> VoxelTypes;

	// Warning: Doesn't initialize values
	FORCEINLINE VOXEL_API void SetSize(int32 NewSizeX, int32 NewSizeY, int32 NewSizeZ);

	FORCEINLINE VOXEL_API float GetValue(const int X, const int Y, const int Z);
	FORCEINLINE VOXEL_API FVoxelMaterial GetMaterial(const int X, const int Y, const int Z);
	FORCEINLINE VOXEL_API EVoxelType GetVoxelType(const int X, const int Y, const int Z);

	FORCEINLINE VOXEL_API void SetValue(const int X, const int Y, const int Z, const float NewValue);
	FORCEINLINE VOXEL_API void SetMaterial(const int X, const int Y, const int Z, const FVoxelMaterial NewMaterial);
	FORCEINLINE VOXEL_API void SetVoxelType(const int X, const int Y, const int Z, const EVoxelType VoxelType);
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, FDecompressedVoxelDataAsset& Asset)
{
	Ar << Asset.SizeX;
	Ar << Asset.SizeY;
	Ar << Asset.SizeZ;

	Ar << Asset.Values;
	Ar << Asset.Materials;

	Ar << Asset.VoxelTypes;

	return Ar;
}


UCLASS(MinimalAPI)
class UVoxelDataAsset : public UObject
{
	GENERATED_BODY()

public:
	UVoxelDataAsset(const FObjectInitializer& ObjectInitializer);

	VOXEL_API void Init(FDecompressedVoxelDataAsset& Asset);
	VOXEL_API bool GetDecompressedAsset(FDecompressedVoxelDataAsset& Asset);

private:
	UPROPERTY()
		TArray<uint8> Data;
};
