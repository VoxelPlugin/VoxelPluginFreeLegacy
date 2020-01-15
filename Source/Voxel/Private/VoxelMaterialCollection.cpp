// Copyright 2020 Phyronnaz

#include "VoxelRender/VoxelMaterialCollection.h"
#include "VoxelRender/VoxelBlendedMaterial.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstanceConstant.h"
#include "UObject/ConstructorHelpers.h"

UMaterialInterface* UVoxelMaterialCollectionBase::GetVoxelMaterial(const FVoxelBlendedMaterialUnsorted& Index, bool bTessellation) const
{
	check(false);
	return nullptr;
}

TMap<FVoxelBlendedMaterialSorted, FVoxelBlendedMaterialUnsorted> UVoxelMaterialCollectionBase::GetBlendedMaterialsMap() const
{
	return {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UMaterialInterface* UVoxelBasicMaterialCollection::GetVoxelMaterial(const FVoxelBlendedMaterialUnsorted& Index, bool bTessellation) const
{
	if (Index.Kind == FVoxelBlendedMaterialUnsorted::Single && Materials.IsValidIndex(Index.Index0))
	{
		auto& Element = Materials[Index.Index0];
		return bTessellation ? Element.TessellatedMaterial : Element.Material;
	}
	else
	{
		return nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UMaterialInterface* FVoxelMaterialCollectionGenerated::GetVoxelMaterial(const FVoxelBlendedMaterialUnsorted& Index) const
{
	switch (Index.Kind)
	{
	case FVoxelBlendedMaterialUnsorted::Single:
	{
		if (!GeneratedSingleMaterials)
		{
			return nullptr;
		}
		UMaterialInterface** Result = GeneratedSingleMaterials->Map.Find(Index.Index0);
		return Result ? *Result : nullptr;
	}
	case FVoxelBlendedMaterialUnsorted::Double:
	{
		if (!GeneratedDoubleMaterials)
		{
			return nullptr;
		}
		UMaterialInterface** Result = GeneratedDoubleMaterials->Map.Find(FVoxelMaterialCollectionDoubleIndex(Index.Index0, Index.Index1));
		return Result ? *Result : nullptr;
	}
	case FVoxelBlendedMaterialUnsorted::Triple:
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

UVoxelMaterialCollection::UVoxelMaterialCollection()
{
    static ConstructorHelpers::FObjectFinder<UMaterial> SingleHelper(TEXT("/Voxel/MaterialHelpers/MF_SingleMaterialTemplate"));
    static ConstructorHelpers::FObjectFinder<UMaterial> DoubleHelper(TEXT("/Voxel/MaterialHelpers/MF_DoubleMaterialTemplate"));
    static ConstructorHelpers::FObjectFinder<UMaterial> TripleHelper(TEXT("/Voxel/MaterialHelpers/MF_TripleMaterialTemplate"));

	SingleMaterialTemplate = SingleHelper.Object;
	DoubleMaterialTemplate = DoubleHelper.Object;
	TripleMaterialTemplate = TripleHelper.Object;

	bShouldGenerateBlendings = true;
}

UMaterialInterface* UVoxelMaterialCollection::GetVoxelMaterial(const FVoxelBlendedMaterialUnsorted& Index, bool bTessellation) const
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

TMap<FVoxelBlendedMaterialSorted, FVoxelBlendedMaterialUnsorted> UVoxelMaterialCollection::GetBlendedMaterialsMap() const
{
	TMap<FVoxelBlendedMaterialSorted, FVoxelBlendedMaterialUnsorted> Result;
	if (GeneratedMaterials.GeneratedDoubleMaterials)
	{
		for (auto& It : GeneratedMaterials.GeneratedDoubleMaterials->SortedIndexMap)
		{
			Result.Add(FVoxelBlendedMaterialSorted(It.Key.I, It.Key.J), FVoxelBlendedMaterialUnsorted(It.Value.I, It.Value.J));
		}
	}
	if (GeneratedMaterials.GeneratedTripleMaterials)
	{
		for (auto& It : GeneratedMaterials.GeneratedTripleMaterials->SortedIndexMap)
		{
			Result.Add(FVoxelBlendedMaterialSorted(It.Key.I, It.Key.J, It.Key.K), FVoxelBlendedMaterialUnsorted(It.Value.I, It.Value.J, It.Value.K));
		}
	}
	return Result;
}

bool UVoxelMaterialCollection::IsValidIndex(uint8 Index) const
{
	return Materials.ContainsByPredicate([&](FVoxelMaterialCollectionElement& Element)
	{
		if (Element.Children.Num() == 0)
		{
			return Element.Index == Index;
		}
		else
		{
			return Element.Children.ContainsByPredicate([&](FVoxelMaterialCollectionElementIndex& Child)
			{
				return Child.InstanceIndex == Index;
			});
		}
	});
}

int32 UVoxelMaterialCollection::GetMaterialIndex(FName Name) const
{
	for (auto& Material : Materials)
	{
		if (Material.MaterialFunction)
		{
			if (Material.MaterialFunction->GetFName() == Name)
			{
				return Material.Index;
			}
			for (auto& Instance : Material.Children)
			{
				if (Instance.MaterialInstance)
				{
					if (Instance.MaterialInstance->GetFName() == Name)
					{
						return Instance.InstanceIndex;
					}
				}
			}
		}
	}
	return -1;
}
