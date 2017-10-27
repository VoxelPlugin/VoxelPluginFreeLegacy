// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelDataAsset.h"
#include "VoxelWorldGenerator.h"
#include "VoxelDataAssetWorldGenerator.generated.h"

// TODO: UBoundedVoxelWorldGenerator

/**
*
*/
UCLASS(Blueprintable)
class VOXEL_API UVoxelDataAssetWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		UVoxelDataAsset* Asset;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UVoxelWorldGenerator> DefaultWorldGenerator;


	UVoxelDataAssetWorldGenerator();


	virtual float GetDefaultValue(int X, int Y, int Z) override;

	virtual FVoxelMaterial GetDefaultMaterial(int X, int Y, int Z) override;

	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld) override;

private:
	UPROPERTY()
		UVoxelWorldGenerator* InstancedWorldGenerator;

	FDecompressedVoxelDataAsset DecompressedAsset;

	void CreateGeneratorAndDecompressedAsset();
};
