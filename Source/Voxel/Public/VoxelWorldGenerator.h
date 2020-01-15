// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelWorldGenerator.generated.h"

class FVoxelWorldGeneratorInstance;
class FVoxelTransformableWorldGeneratorInstance;

/**
 * A UVoxelWorldGenerator is used to create a FVoxelWorldGeneratorInstance
 */
UCLASS(Blueprintable, Abstract)
class VOXEL_API UVoxelWorldGenerator : public UObject
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelWorldGenerator Interface
	virtual TMap<FName, int32> GetDefaultSeeds() const;
	virtual TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetInstance();
	//~ End UVoxelWorldGenerator Interface
};

// World generator that can be moved around
UCLASS(Blueprintable, Abstract)
class VOXEL_API UVoxelTransformableWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelWorldGeneratorWithTransform Interface
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance();
	// Will be only called on the CDO. Must not save the transform
	virtual void SaveInstance(const FVoxelTransformableWorldGeneratorInstance& Instance, FArchive& Ar) const;
	// Will be only called on the CDO
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> LoadInstance(FArchive& Ar) const;
	//~ End UVoxelWorldGeneratorWithTransform Interface
	
	virtual TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetInstance() override final;
};