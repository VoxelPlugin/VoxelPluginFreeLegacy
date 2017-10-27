// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelLandscapeAsset.generated.h"

struct FDecompressedVoxelLandscapeAsset
{
	TArray<float> Heights;
	TArray<FVoxelMaterial> Materials;
	int Size;

	int Precision;
	float HardnessMultiplier;


	FORCEINLINE int GetLowerBound(int X, int Y, const float VoxelSize);
	FORCEINLINE int GetUpperBound(int X, int Y, const float VoxelSize);

	FORCEINLINE float GetValue(const int X, const int Y, const int Z, const float VoxelSize);
	FORCEINLINE FVoxelMaterial GetMaterial(const int X, const int Y, const int Z, const float VoxelSize);
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, FDecompressedVoxelLandscapeAsset& Asset)
{
	Ar << Asset.Heights;
	Ar << Asset.Materials;
	Ar << Asset.Size;

	return Ar;
}

/**
 *
 */
UCLASS(MinimalAPI)
class UVoxelLandscapeAsset : public UObject
{
	GENERATED_BODY()

public:
	UVoxelLandscapeAsset(const FObjectInitializer& ObjectInitializer);

	VOXEL_API void Init(TArray<float>& Heights, TArray<FVoxelMaterial>& Materials, int Size);
	VOXEL_API bool GetDecompressedAsset(FDecompressedVoxelLandscapeAsset& Asset);

private:
	// Higher precision can improve render quality, but voxel values are lower (hardness not constant)
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
		int Precision;

	UPROPERTY(EditAnywhere)
		float HardnessMultiplier;

	UPROPERTY()
		TArray<uint8> Data;
};