// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelSpawnerOutputsConfig.generated.h"

UCLASS()
class VOXEL_API UVoxelSpawnerOutputsConfig : public UObject
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelSpawnerOutputConfig Interface
	virtual TArray<FName> GetFloatOutputs() const { unimplemented(); return {}; }
	//~ End UVoxelSpawnerOutputConfig Interface
};