// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelLegacySpawnerGroup.generated.h"

class UVoxelMeshSpawner;

USTRUCT()
struct FVoxelSpawnerGroupChild
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	UVoxelMeshSpawner* Spawner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = 0, ClampMax = 1, UIMin = 0, UIMax = 1))
	float Probability = 0;
};

UCLASS()
class VOXEL_API UVoxelSpawnerGroup : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bNormalizeProbabilitiesOnEdit = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FVoxelSpawnerGroupChild> Children;
};