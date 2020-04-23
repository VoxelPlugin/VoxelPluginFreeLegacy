// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterialCollectionBase.generated.h"

struct FVoxelMaterialIndices;
class UMaterialInterface;

UCLASS(Abstract)
class VOXEL_API UVoxelMaterialCollectionBase : public UObject
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelMaterialCollectionBase Interface
	// Max number of material indices this collection can handle
	// eg if = 2, this collection can only blend between 2 indices at a time
	virtual int32 GetMaxMaterialIndices() const
	{
		unimplemented();
		return 0;
	}
	virtual UMaterialInterface* GetVoxelMaterial(const FVoxelMaterialIndices& Indices, bool bTessellation, uint64 UniqueIdForErrors) const
	{
		unimplemented();
		return nullptr;
	}
	// Get the material index from a material name
	virtual int32 GetMaterialIndex(FName Name) const
	{
		return -1;	
	}
	virtual void ClearCache()
	{
	}
	//~ End UVoxelMaterialCollectionBase Interface
};