// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSpawner.h"
#include "VoxelSpawnerGroup.generated.h"

class UVoxelSpawnerGroup;


USTRUCT()
struct FVoxelSpawnerGroupChild
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	UVoxelSpawner* Spawner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = 0, ClampMax = 1, UIMin = 0, UIMax = 1))
	float Probability = 0;
};

UCLASS()
class VOXEL_API UVoxelSpawnerGroup : public UVoxelSpawner
{
	GENERATED_BODY()

public:
	// Probabilities do not need to be normalized, although it might be harder to understand what's happening if they're not
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bNormalizeProbabilitiesOnEdit = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FVoxelSpawnerGroupChild> Children;
	

protected:
	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface
};