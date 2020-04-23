// Copyright 2020 Phyronnaz

#include "VoxelRender/MaterialCollections/VoxelMaterialCollection.h"
#include "VoxelRender/VoxelMaterialIndices.h"
#include "VoxelUniqueError.h"
#include "VoxelMessages.h"

int32 UDEPRECATED_VoxelMaterialCollection::GetMaxMaterialIndices() const
{
	return 3;
}

UMaterialInterface* UDEPRECATED_VoxelMaterialCollection::GetVoxelMaterial(const FVoxelMaterialIndices& Indices, bool bTessellation, uint64 UniqueIdForErrors) const
{
	static TVoxelUniqueError<> UniqueError;
	FVoxelMessages::CondError(UniqueError(UniqueIdForErrors, {}),
		"Material Collections are deprecated, use Instanced Material Collections instead",
		this);
	return nullptr;
}

int32 UDEPRECATED_VoxelMaterialCollection::GetMaterialIndex(FName Name) const
{
	return -1;
}