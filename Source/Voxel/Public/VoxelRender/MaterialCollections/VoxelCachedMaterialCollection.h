// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRender/VoxelMaterialIndices.h"
#include "VoxelRender/MaterialCollections/VoxelMaterialCollectionBase.h"
#include "VoxelCachedMaterialCollection.generated.h"

UCLASS(Abstract)
class VOXEL_API UVoxelCachedMaterialCollection : public UVoxelMaterialCollectionBase
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelMaterialCollectionBase Interface
	virtual UMaterialInterface* GetVoxelMaterial(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const override final;
	virtual void InitializeCollection() override;
	//~ End UVoxelMaterialCollectionBase Interface
	
	//~ Begin UVoxelCachedMaterialCollection Interface
	virtual UMaterialInterface* GetVoxelMaterial_NotCached(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const
	{
		unimplemented();
		return nullptr;
	}
	//~ End UVoxelCachedMaterialCollection Interface

private:
	UPROPERTY(Transient)
	mutable TMap<FVoxelMaterialIndices, UMaterialInterface*> CachedMaterials;
};