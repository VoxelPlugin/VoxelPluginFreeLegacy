// Copyright 2020 Phyronnaz

#include "VoxelRender/MaterialCollections/VoxelBasicMaterialCollection.h"
#include "VoxelRender/VoxelMaterialIndices.h"
#include "VoxelMaterialUtilities.h"
#include "VoxelUniqueError.h"
#include "VoxelMessages.h"

#include "Materials/Material.h"

int32 UVoxelBasicMaterialCollection::GetMaxMaterialIndices() const
{
	return 1;
}

UMaterialInterface* UVoxelBasicMaterialCollection::GetVoxelMaterial(const FVoxelMaterialIndices& Indices, bool bTessellation, uint64 UniqueIdForErrors) const
{
	ensure(Indices.NumIndices == 1);
	const int32 Index = Indices.SortedIndices[0];

	UMaterialInterface* MaterialInterface = nullptr;
	if (auto* Layer = Layers.FindByKey(Index))
	{
		MaterialInterface = bTessellation ? Layer->LayerTessellatedMaterial : Layer->LayerMaterial;
	}

	if (!MaterialInterface)
	{
		if (bTessellation)
		{
			static TVoxelUniqueError<uint8> UniqueError;
			FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
				UniqueError(UniqueIdForErrors, Index),
				FString::Printf(TEXT("Missing tessellated material for index %d"), Index),
				this);
		}
		else
		{
			static TVoxelUniqueError<uint8> UniqueError;
			FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
				UniqueError(UniqueIdForErrors, Index),
				FString::Printf(TEXT("Missing material for index %d"), Index),
				this);
		}
	}

	if (MaterialInterface && FVoxelUtilities::IsMaterialTessellated(MaterialInterface) != bTessellation)
	{
		if (bTessellation)
		{
			static TVoxelUniqueError<uint8> UniqueError;
			FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
				UniqueError(UniqueIdForErrors, Index),
				FString::Printf(TEXT("Tessellated material for index %d has tessellation disabled!"), Index),
				this);
		}
		else
		{
			static TVoxelUniqueError<uint8> UniqueError;
			FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
				UniqueError(UniqueIdForErrors, Index),
				FString::Printf(TEXT("Material for index %d has tessellation enabled! Please use Tessellated Material for that instead and enable tessellation in your voxel world settings"), Index),
				this);
		}
		MaterialInterface = nullptr;
	}

	return MaterialInterface;
}

int32 UVoxelBasicMaterialCollection::GetMaterialIndex(FName Name) const
{
	for (auto& Layer : Layers)
	{
		if (Layer.LayerMaterial && Layer.LayerMaterial->GetFName() == Name)
		{
			return Layer.LayerIndex;
		}
		if (Layer.LayerTessellatedMaterial && Layer.LayerTessellatedMaterial->GetFName() == Name)
		{
			return Layer.LayerIndex;
		}
	}
	return -1;
}

#if WITH_EDITOR
void UVoxelBasicMaterialCollection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		TSet<int32> Indices;
		for (auto& Layer : Layers)
		{
			bool bIsAlreadyInSet = true;
			while (bIsAlreadyInSet)
			{
				Indices.Add(Layer.LayerIndex, &bIsAlreadyInSet);
				if (bIsAlreadyInSet) Layer.LayerIndex++;
			}
		}
	}
}
#endif