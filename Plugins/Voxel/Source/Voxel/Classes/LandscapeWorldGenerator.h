// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "VoxelLandscapeAsset.h"
#include "LandscapeWorldGenerator.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class VOXEL_API ULandscapeWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		UVoxelLandscapeAsset* Landscape;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UVoxelWorldGenerator> DefaultWorldGenerator;



	ULandscapeWorldGenerator();


	virtual float GetDefaultValue(int X, int Y, int Z) override;

	virtual FVoxelMaterial GetDefaultMaterial(int X, int Y, int Z) override;

	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld) override;

private:
	UVoxelWorldGenerator* InstancedWorldGenerator;
	FDecompressedVoxelLandscapeAsset Asset;
	int VoxelSize;

	FCriticalSection CreateGeneratorLock;

	void CreateGeneratorAndDecompressedAsset();
};
