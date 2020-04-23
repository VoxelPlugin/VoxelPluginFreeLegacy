// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRender/VoxelMaterialIndices.h"
#include "VoxelRender/MaterialCollections/VoxelMaterialCollectionBase.h"
#include "VoxelCachedMaterialCollection.generated.h"

USTRUCT()
struct FVoxelCachedMaterialCollectionKey
{
	GENERATED_BODY()

	FVoxelMaterialIndices Indices;
	bool bTessellation = false;

	inline bool operator==(const FVoxelCachedMaterialCollectionKey& Other) const
	{
		return Indices == Other.Indices && bTessellation == Other.bTessellation;
	}
};

inline uint32 GetTypeHash(const FVoxelCachedMaterialCollectionKey& Key)
{
	return GetTypeHash(Key.Indices) * 2 + Key.bTessellation;
}

UCLASS(Abstract)
class VOXEL_API UVoxelCachedMaterialCollection : public UVoxelMaterialCollectionBase
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelMaterialCollectionBase Interface
	virtual UMaterialInterface* GetVoxelMaterial(const FVoxelMaterialIndices& Indices, bool bTessellation, uint64 UniqueIdForErrors) const override final;
	virtual void ClearCache() override final;
	//~ End UVoxelMaterialCollectionBase Interface
	
	//~ Begin UVoxelCachedMaterialCollection Interface
	virtual UMaterialInterface* GetVoxelMaterial_NotCached(const FVoxelMaterialIndices& Indices, bool bTessellation, uint64 UniqueIdForErrors) const
	{
		unimplemented();
		return nullptr;
	}
	//~ End UVoxelCachedMaterialCollection Interface

private:
	UPROPERTY(Transient)
	mutable TMap<FVoxelCachedMaterialCollectionKey, UMaterialInterface*> CachedMaterials;
};