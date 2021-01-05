// Copyright 2021 Phyronnaz

#include "VoxelRender/MaterialCollections/VoxelBasicMaterialCollection.h"
#include "VoxelRender/VoxelMaterialIndices.h"
#include "VoxelUniqueError.h"
#include "VoxelMessages.h"

#include "Materials/Material.h"

int32 UVoxelBasicMaterialCollection::GetMaxMaterialIndices() const
{
	return 1;
}

UMaterialInterface* UVoxelBasicMaterialCollection::GetVoxelMaterial(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const
{
	ensure(Indices.NumIndices == 1);
	const int32 Index = Indices.SortedIndices[0];

	UMaterialInterface* MaterialInterface = nullptr;
	if (auto* Layer = Layers.FindByKey(Index))
	{
		MaterialInterface = Layer->LayerMaterial;
	}

	if (!MaterialInterface)
	{
		static TVoxelUniqueError<uint8> UniqueError;
		FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
			UniqueError(UniqueIdForErrors, Index),
			FString::Printf(TEXT("Missing material for index %d"), Index),
			this);
	}

	return MaterialInterface;
}

TArray<FVoxelMaterialCollectionMaterialInfo> UVoxelBasicMaterialCollection::GetMaterials() const
{
	TArray<FVoxelMaterialCollectionMaterialInfo> Result;
	for (const auto& Layer : Layers)
	{
		Result.Add(FVoxelMaterialCollectionMaterialInfo{ Layer.LayerIndex, Layer.LayerMaterial });
	}
	return Result;
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