// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelSpawners/VoxelBasicSpawner.h"
#include "VoxelWorldGeneratorPicker.h"
#include "VoxelAssetSpawner.generated.h"

class UVoxelAssetSpawner;
class UVoxelTransformableWorldGenerator;
class FVoxelTransformableWorldGeneratorInstance;
class FVoxelAssetSpawnerProxy;


UCLASS()
class VOXEL_API UVoxelAssetSpawner : public UVoxelBasicSpawner
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVoxelTransformableWorldGeneratorPicker Generator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FIntBox GeneratorLocalBounds = FIntBox(-25, 25);

	// The voxel world seeds will be sent to the generator.
	// Add the names of the seeds you want to be randomized here
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<FName> Seeds;

	// All generators are created at begin play
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = 1))
	int32 NumberOfDifferentSeedsToUse = 1;

	// Priority of the spawned assets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bRoundAssetPosition = false;
	
public:
	//~ Begin UVoxelSpawner Interface
};