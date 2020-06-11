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
	virtual UMaterialInterface* GetVoxelMaterial(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const
	{
		unimplemented();
		return nullptr;
	}
	virtual UMaterialInterface* GetVoxelMaterialForPreview(uint8 Index) const
	{
		return nullptr;
	}
	// Used by paint material customization. Some materials might be null.
	virtual TMap<int32, UMaterialInterface*> GetVoxelMaterials() const
	{
		return {};
	}
	// Get the material index from a material name
	virtual int32 GetMaterialIndex(FName Name) const
	{
		return -1;	
	}
	// Called before the material collection is used (can be at runtime when dynamic renderer settings change)
	virtual void InitializeCollection()
	{
	}
	//~ End UVoxelMaterialCollectionBase Interface
};