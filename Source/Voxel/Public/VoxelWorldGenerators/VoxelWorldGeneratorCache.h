// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelWorldGeneratorInit.h"
#include "UObject/GCObject.h"

class UVoxelWorldGenerator;
class FVoxelWorldGeneratorInstance;

class VOXEL_API FVoxelWorldGeneratorCache : public FGCObject
{
public:
	explicit FVoxelWorldGeneratorCache(const FVoxelWorldGeneratorInit& WorldGeneratorInit);
	
	// Will create & init a new instance if needed, or reuse a cached one
	TVoxelSharedRef<FVoxelWorldGeneratorInstance> CreateWorldGeneratorInstance(UVoxelWorldGenerator& Generator);

protected:
	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface

private:
	FVoxelWorldGeneratorInit WorldGeneratorInit;
	TMap<TWeakObjectPtr<UVoxelWorldGenerator>, TVoxelSharedPtr<FVoxelWorldGeneratorInstance>> WorldGeneratorCache;
};