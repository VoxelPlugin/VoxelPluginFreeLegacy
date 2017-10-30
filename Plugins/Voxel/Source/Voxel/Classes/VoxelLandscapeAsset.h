// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelAsset.h"
#include "VoxelLandscapeAsset.generated.h"

// Center: bottom center
struct VOXEL_API FDecompressedVoxelLandscapeAsset : FDecompressedVoxelAsset
{
	TArray<float> Heights;
	TArray<FVoxelMaterial> Materials;
	int Size;

	// Will be set by UVoxelLandscapeAsset
	int Precision;
	float HardnessMultiplier;
	float VoxelSize;

	float GetValue(const int X, const int Y, const int Z) override;
	FVoxelMaterial GetMaterial(const int X, const int Y, const int Z) override;
	FVoxelType GetVoxelType(const int X, const int Y, const int Z) override;
	FVoxelBox GetBounds() override;
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
class UVoxelLandscapeAsset : public UVoxelAsset
{
	GENERATED_BODY()

public:
	UVoxelLandscapeAsset(const FObjectInitializer& ObjectInitializer);

	bool GetDecompressedAsset(FDecompressedVoxelAsset*& Asset, const float VoxelSize) override;

protected:
	void AddAssetToArchive(FBufferArchive& ToBinary, FDecompressedVoxelAsset* Asset) override;
	void GetAssetFromArchive(FMemoryReader& FromBinary, FDecompressedVoxelAsset* Asset) override;

private:
	// Higher precision can improve render quality, but voxel values are lower (hardness not constant)
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
		int Precision;

	UPROPERTY(EditAnywhere)
		float HardnessMultiplier;
};