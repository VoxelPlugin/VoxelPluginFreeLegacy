// Copyright 2019 Phyronnaz

#include "VoxelMaterialCollection.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "Materials/MaterialInstanceDynamic.h"

UMaterialInterface* FVoxelMaterialCollectionGenerated::GetVoxelMaterial(const FVoxelBlendedMaterial& Index)
{
	switch (Index.Kind)
	{
	case FVoxelBlendedMaterial::Single:
	{
		if (!GeneratedSingleMaterials)
		{
			return nullptr;
		}
		UMaterialInterface** Result = GeneratedSingleMaterials->Map.Find(Index.Index0);
		return Result ? *Result : nullptr;
	}
	case FVoxelBlendedMaterial::Double:
	{
		if (!GeneratedDoubleMaterials)
		{
			return nullptr;
		}
		UMaterialInterface** Result = GeneratedDoubleMaterials->Map.Find(FVoxelMaterialCollectionDoubleIndex(Index.Index0, Index.Index1));
		return Result ? *Result : nullptr;
	}
	case FVoxelBlendedMaterial::Triple:
	{
		if (!GeneratedTripleMaterials)
		{
			return nullptr;
		}
		UMaterialInterface** Result = GeneratedTripleMaterials->Map.Find(FVoxelMaterialCollectionTripleIndex(Index.Index0, Index.Index1, Index.Index2));
		return Result ? *Result : nullptr;
	}
	default:
	{
		check(false);
		return nullptr;
	}
	}
}

UMaterialInterface* UVoxelMaterialCollection::GetVoxelMaterial(const FVoxelBlendedMaterial& Index, bool bTessellation)
{
	if (bTessellation)
	{
		return GeneratedMaterialsTess.GetVoxelMaterial(Index);
	}
	else
	{
		return GeneratedMaterials.GetVoxelMaterial(Index);
	}
}

void UVoxelMaterialCollection::PostLoad()
{
	Super::PostLoad();

	if (MaterialFunctions_DEPRECATED.Num() > 0)
	{
		for (int32 Index = 0; Index < MaterialFunctions_DEPRECATED.Num(); Index++)
		{
			int32 Last = Materials.Emplace();
			auto& NewMaterial = Materials[Last];
			NewMaterial.Index = Index;
			NewMaterial.MaterialFunction = MaterialFunctions_DEPRECATED[Index];
			if (PhysicalMaterials_DEPRECATED.IsValidIndex(Index))
			{
				NewMaterial.PhysicalMaterial = PhysicalMaterials_DEPRECATED[Index];
			}
		}
	}

	InitVariables();
}

void UVoxelMaterialCollection::PostInitProperties()
{
	Super::PostInitProperties();

	if (!HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		InitVariables();
	}
}

void UVoxelMaterialCollection::InitVariables()
{
	if (!SingleMaterialTemplate)
	{
		Modify();
		SingleMaterialTemplate = LoadObject<UMaterial>(this, TEXT("/Voxel/MaterialHelpers/MF_SingleMaterialTemplate"));
	}
	if (!DoubleMaterialTemplate)
	{
		Modify();
		DoubleMaterialTemplate = LoadObject<UMaterial>(this, TEXT("/Voxel/MaterialHelpers/MF_DoubleMaterialTemplate"));
	}
	if (!TripleMaterialTemplate)
	{
		Modify();
		TripleMaterialTemplate = LoadObject<UMaterial>(this, TEXT("/Voxel/MaterialHelpers/MF_TripleMaterialTemplate"));
	}
}
