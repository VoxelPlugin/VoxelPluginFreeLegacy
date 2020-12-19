// Copyright 2020 Phyronnaz

#include "VoxelFoliage/VoxelFoliage.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

void UVoxelFoliage::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	if (!bDuplicateForPIE)
	{
		Guid = FGuid::NewGuid();
	}
}

#if WITH_EDITOR
void UVoxelFoliage::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	if (PropertyAboutToChange && 
		PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_STATIC(FVoxelInstancedMeshKey, Mesh) && 
		MeshKey.Mesh)
	{
		const TArray<FStaticMaterial>& StaticMaterials = MeshKey.Mesh->StaticMaterials;
		TArray<UMaterialInterface*>& Materials = MeshKey.Materials;
		
		if (StaticMaterials.Num() >= Materials.Num())
		{
			bool bNoChange = true;
			for (int32 Index = 0; Index < Materials.Num(); Index++)
			{
				if (Materials[Index] != StaticMaterials[Index].MaterialInterface)
				{
					bNoChange = false;
					break;
				}
			}

			if (bNoChange)
			{
				Materials.Reset();
			}
		}
	}
}

void UVoxelFoliage::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (MeshKey.Mesh)
	{
		const TArray<FStaticMaterial>& StaticMaterials = MeshKey.Mesh->StaticMaterials;
		if (MeshKey.Materials.Num() < StaticMaterials.Num())
		{
			MeshKey.Materials.SetNum(StaticMaterials.Num());
		}

		for (int32 Index = 0; Index < StaticMaterials.Num(); Index++)
		{
			if (!MeshKey.Materials[Index])
			{
				MeshKey.Materials[Index] = StaticMaterials[Index].MaterialInterface;
			}
		}
	}
}
#endif

#if WITH_EDITOR
bool UVoxelFoliage::NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const
{
	if (Object == this)
	{
		return true;
	}
	if (MeshKey.Mesh == Object)
	{
		return true;
	}
	if (Object == OutputPickerGenerator.GetObject())
	{
		return true;
	}

	for (const FVoxelFoliageDensity& Density : Densities)
	{
		if (Density.NeedsToRebuild(Object, PropertyChangedEvent))
		{
			return true;
		}
	}

	return false;
}
#endif
