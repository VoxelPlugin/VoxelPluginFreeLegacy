// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelUtilities/VoxelMaterialUtilities.h"

#include "Engine/EngineTypes.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"

bool FVoxelUtilities::IsMaterialTessellated(UMaterialInterface* Material)
{
	return false;
}

UMaterialInterface* FVoxelUtilities::GetDefaultMaterial(int32 NumIndices)
{
	static TWeakObjectPtr<UMaterialInterface> Materials[7];

	check(0 <= NumIndices && NumIndices <= 6);
	auto& Material = Materials[NumIndices];

	if (Material.IsValid())
	{
		return Material.Get();
	}
	
	UMaterial* Parent = UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface);

	Material = UMaterialInstanceDynamic::Create(Parent, nullptr);
	check(Material.IsValid());

	Material->AddToRoot();
	return Material.Get();
}