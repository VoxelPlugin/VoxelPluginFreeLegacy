// Copyright 2020 Phyronnaz

#include "VoxelMaterialUtilities.h"

#include "Engine/EngineTypes.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"

bool FVoxelUtilities::IsMaterialTessellated(UMaterialInterface* Material)
{
	UMaterial* BaseMaterial = Material->GetMaterial();
	if (!ensure(BaseMaterial)) return false;
	return BaseMaterial->D3D11TessellationMode != EMaterialTessellationMode::MTM_NoTessellation;
}

UMaterialInterface* FVoxelUtilities::GetDefaultMaterial(bool bTessellation, int32 NumIndices)
{
	static TWeakObjectPtr<UMaterialInterface> Materials[2][7];

	check(0 <= NumIndices && NumIndices <= 6);
	auto& Material = Materials[bTessellation][NumIndices];

	if (Material.IsValid())
	{
		return Material.Get();
	}
	
	UMaterial* Parent;
	if (bTessellation)
	{
		Parent = LoadObject<UMaterial>(nullptr, TEXT("/Voxel/MaterialHelpers/WorldGridMaterial_Tessellated"));
	}
	else
	{
		Parent = UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface);
	}

	Material = UMaterialInstanceDynamic::Create(Parent, nullptr);
	check(Material.IsValid());

	Material->AddToRoot();
	return Material.Get();
}