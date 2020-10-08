// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelGeneratorInstanceWrapper.generated.h"

class UVoxelGenerator;
class FVoxelGeneratorInstance;
class FVoxelTransformableGeneratorInstance;

UCLASS(BlueprintType)
class VOXEL_API UVoxelGeneratorInstanceWrapper : public UObject
{
	GENERATED_BODY()
	
public:
	TVoxelSharedPtr<FVoxelGeneratorInstance> Instance;

	UFUNCTION(BlueprintCallable, Category = "Voxel|Generators")
	bool IsValid() const { return Instance.IsValid(); }
};

UCLASS(BlueprintType)
class VOXEL_API UVoxelTransformableGeneratorInstanceWrapper : public UObject
{
	GENERATED_BODY()
	
public:
	TVoxelSharedPtr<FVoxelTransformableGeneratorInstance> Instance;

	UFUNCTION(BlueprintCallable, Category = "Voxel|Generators")
	bool IsValid() const { return Instance.IsValid(); }
};