// Copyright 2021 Phyronnaz

#include "VoxelFoliage/VoxelFoliage.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
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
		PropertyAboutToChange->GetFName() == "Mesh")
	{
		for (FVoxelFoliageMesh& Mesh : Meshes)
		{
			if (!Mesh.Mesh)
			{
				continue;
			}
			
			const TArray<FStaticMaterial>& StaticMaterials = Mesh.Mesh->StaticMaterials;
			if (StaticMaterials.Num() < Mesh.Materials.Num())
			{
				continue;
			}
		
			bool bNoChange = true;
			for (int32 Index = 0; Index < Mesh.Materials.Num(); Index++)
			{
				if (Mesh.Materials[Index] != StaticMaterials[Index].MaterialInterface)
				{
					bNoChange = false;
					break;
				}
			}

			if (bNoChange)
			{
				Mesh.Materials.Reset();
			}
		}
	}
}

void UVoxelFoliage::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		for (FVoxelFoliageMesh& Mesh : Meshes)
		{
			if (!Mesh.Mesh)
			{
				continue;
			}
			
			const TArray<FStaticMaterial>& StaticMaterials = Mesh.Mesh->StaticMaterials;
			if (Mesh.Materials.Num() < StaticMaterials.Num())
			{
				Mesh.Materials.SetNum(StaticMaterials.Num());
			}

			for (int32 Index = 0; Index < StaticMaterials.Num(); Index++)
			{
				if (!Mesh.Materials[Index])
				{
					Mesh.Materials[Index] = StaticMaterials[Index].MaterialInterface;
				}
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
