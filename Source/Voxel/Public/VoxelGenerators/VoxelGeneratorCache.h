// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Templates/SubclassOf.h"
#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGeneratorCache.generated.h"

class UVoxelGeneratorInstanceWrapper;
class UVoxelTransformableGeneratorInstanceWrapper;

UCLASS(BlueprintType)
class VOXEL_API UVoxelGeneratorCache : public UObject
{
public:
	GENERATED_BODY()

public:
	/**
	 * Creates (or reuse if possible) a new generator instance
	 *
	 * Among other things, this is required for DataItemActors to reuse generators, which allows for smaller update when moving them
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	UVoxelGeneratorInstanceWrapper* MakeGeneratorInstance(FVoxelGeneratorPicker Picker) const;
	
	/**
	 * Creates (or reuse if possible) a new generator instance
	 *
	 * Among other things, this is required for DataItemActors to reuse generators, which allows for smaller update when moving them
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	UVoxelTransformableGeneratorInstanceWrapper* MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker) const;

public:
	void SetGeneratorInit(const FVoxelGeneratorInit& NewInit)
	{
		GeneratorInit = NewInit;
	}
	void ClearCache()
	{
		Cache.Reset();
	}

private:
	UPROPERTY()
	FVoxelGeneratorInit GeneratorInit;
	
	UPROPERTY()
	mutable TMap<FVoxelGeneratorPicker, UVoxelGeneratorInstanceWrapper*> Cache;
	
	UPROPERTY()
	mutable TMap<FVoxelTransformableGeneratorPicker, UVoxelTransformableGeneratorInstanceWrapper*> TransformableCache;
};