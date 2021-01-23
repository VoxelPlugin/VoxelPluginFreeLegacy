// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelBasicSpawner.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelAssetSpawner.generated.h"

class UVoxelAssetSpawner;
class UVoxelTransformableGenerator;
class FVoxelAssetSpawnerProxy;

UCLASS()
class VOXELLEGACYSPAWNERS_API UVoxelAssetSpawner : public UVoxelBasicSpawner
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Settings")
	FVoxelTransformableGeneratorPicker Generator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Settings")
	FVoxelIntBox GeneratorLocalBounds = FVoxelIntBox(-25, 25);

	// The voxel world seeds will be sent to the generator.
	// Add the names of the seeds you want to be randomized here
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Settings")
	TArray<FName> Seeds;

	// All generators are created at begin play
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Settings", meta = (ClampMin = 1))
	int32 NumberOfDifferentSeedsToUse = 1;

	// Priority of the spawned assets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Settings")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Settings")
	bool bRoundAssetPosition = false;
};