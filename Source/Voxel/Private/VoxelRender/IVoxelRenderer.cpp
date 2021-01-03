// Copyright 2021 Phyronnaz

#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelMaterialIndices.h"
#include "VoxelRender/MaterialCollections/VoxelMaterialCollectionBase.h"
#include "VoxelMessages.h"
#include "VoxelUniqueError.h"
#include "VoxelUtilities/VoxelMaterialUtilities.h"

UMaterialInterface* IVoxelRenderer::GetVoxelMaterial(int32 LOD, const FVoxelMaterialIndices& MaterialIndices) const
{
	auto* MaterialCollection = DynamicSettings->GetLODMaterialSettings(LOD).MaterialCollection.Get();
	if (!MaterialCollection)
	{
		static TVoxelUniqueError<> UniqueError;
		FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
			UniqueError(UniqueId, {}),
			"Invalid Material Collection",
			Settings.Owner.Get());
		return FVoxelUtilities::GetDefaultMaterial(MaterialIndices.NumIndices);
	}
	
	return MaterialCollection->GetVoxelMaterial(MaterialIndices, UniqueId);
}

UMaterialInterface* IVoxelRenderer::GetVoxelMaterial(int32 LOD) const
{
	if (auto* Material = DynamicSettings->GetLODMaterialSettings(LOD).Material.Get())
	{
		return Material;
	}
	else
	{
		static TVoxelUniqueError<> UniqueError;
		FVoxelMessages::CondError(
			UniqueError(UniqueId, {}),
			"Invalid VoxelMaterial",
			Settings.Owner.Get());
		return FVoxelUtilities::GetDefaultMaterial(0);
	}
}

uint64 IVoxelRenderer::UniqueIdCounter = 0;