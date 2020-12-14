// Copyright 2020 Phyronnaz

#include "VoxelSpawners/VoxelFoliage.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

FVoxelInstancedMeshKey UVoxelFoliage::GetMeshKey() const
{
	return UVoxelBlueprintLibrary::MakeInstancedMeshKey(StaticMesh, ActorClass, Materials, InstancedMeshSettings);	
}

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
		PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_STATIC(UVoxelFoliage, StaticMesh) && 
		StaticMesh)
	{
		const TArray<FStaticMaterial>& StaticMaterials = StaticMesh->StaticMaterials;

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

	if (StaticMesh)
	{
		const TArray<FStaticMaterial>& StaticMaterials = StaticMesh->StaticMaterials;
		if (Materials.Num() < StaticMaterials.Num())
		{
			Materials.SetNum(StaticMaterials.Num());
		}

		for (int32 Index = 0; Index < StaticMaterials.Num(); Index++)
		{
			if (!Materials[Index])
			{
				Materials[Index] = StaticMaterials[Index].MaterialInterface;
			}
		}
	}
}
#endif

#if WITH_EDITOR
bool UVoxelFoliage::NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const
{
	if (StaticMesh == Object)
	{
		return true;
	}
	if (Object == MainGeneratorForDropdowns.GetObject())
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
