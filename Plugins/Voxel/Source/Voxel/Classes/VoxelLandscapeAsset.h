// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelLandscapeAsset.generated.h"

struct FDecompressedVoxelLandscapeAsset
{
public:
	TArray<float> Heights;
	TArray<FVoxelMaterial> Materials;
	int Size;

	int Precision;
	float HardnessMultiplier;


	FORCEINLINE int GetLowerBound(int X, int Y, const float VoxelSize);
	FORCEINLINE int GetUpperBound(int X, int Y, const float VoxelSize);

	FORCEINLINE float GetValue(const int X, const int Y, const int Z, const float VoxelSize);
	FORCEINLINE FVoxelMaterial GetMaterial(const int X, const int Y, const int Z, const float VoxelSize);

private:
	;
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


	UPROPERTY()
		TArray<uint8> Data;

	void Init(TArray<float>& Heights, TArray<FVoxelMaterial>& Materials, int Size);

	bool GetDecompressedAsset(FDecompressedVoxelLandscapeAsset& Asset);

private:
	// Higher precision can improve render quality, but voxel values are lower (hardness not constant)
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
		int Precision;

	UPROPERTY(EditAnywhere)
		float HardnessMultiplier;
};