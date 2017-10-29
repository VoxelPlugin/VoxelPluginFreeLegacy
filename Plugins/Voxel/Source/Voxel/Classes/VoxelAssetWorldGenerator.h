// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelDataAsset.h"
#include "VoxelWorldGenerator.h"
#include "VoxelAssetWorldGenerator.generated.h"

// TODO: UBoundedVoxelWorldGenerator

/**
*
*/
UCLASS(Blueprintable)
class VOXEL_API UVoxelAssetWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		UVoxelAsset* Asset;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UVoxelWorldGenerator> DefaultWorldGenerator;


	UVoxelAssetWorldGenerator();


	virtual float GetDefaultValue(int X, int Y, int Z) override;

	virtual FVoxelMaterial GetDefaultMaterial(int X, int Y, int Z) override;

	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld) override;

private:
	UPROPERTY()
		UVoxelWorldGenerator* InstancedWorldGenerator;

	FDecompressedVoxelAsset* DecompressedAsset;
	FVoxelBox Bounds;

	void CreateGeneratorAndDecompressedAsset(const float VoxelSize);
};
