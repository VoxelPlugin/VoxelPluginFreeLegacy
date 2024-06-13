// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelRender/MaterialCollections/VoxelLandscapeMaterialCollection.h"
#include "VoxelRender/VoxelMaterialIndices.h"
#include "VoxelRender/VoxelMaterialExpressions.h"
#include "VoxelMessages.h"
#include "VoxelEditorDelegates.h"

#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"

UMaterialInterface* UVoxelLandscapeMaterialCollection::GetVoxelMaterial(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const
{
	if (!Material)
	{
		return nullptr;
	}

	FVoxelLandscapeMaterialCollectionPermutation Permutation;
	for (int32 Index = 0; Index < Indices.NumIndices; Index++)
	{
		Permutation.Names[Index] = IndicesToLayers.FindRef(Indices.SortedIndices[Index]).Name;
	}
	
	return FindOrAddPermutation(Permutation);
}

UMaterialInterface* UVoxelLandscapeMaterialCollection::GetIndexMaterial(uint8 Index) const
{
	for (auto& Layer : Layers)
	{
		if (Layer.Index == Index)
		{
			FVoxelLandscapeMaterialCollectionPermutation Permutation;
			Permutation.Names[0] = Layer.Name;
			return FindOrAddPermutation(Permutation);
		}
	}
	return nullptr;
}

TArray<UVoxelMaterialCollectionBase::FMaterialInfo> UVoxelLandscapeMaterialCollection::GetMaterials() const
{
	TArray<FMaterialInfo> Result;
	for (auto& Layer : Layers)
	{
		FVoxelLandscapeMaterialCollectionPermutation Permutation;
		Permutation.Names[0] = Layer.Name;
		Result.Add(FMaterialInfo{ Layer.Index,Layer.Name,  FindOrAddPermutation(Permutation) });
	}
	return Result;
}

int32 UVoxelLandscapeMaterialCollection::GetMaterialIndex(FName Name) const
{
	for (auto& Layer : Layers)
	{
		if (Layer.Name == Name)
		{
			return Layer.Index;
		}
	}
	return -1;
}

void UVoxelLandscapeMaterialCollection::InitializeCollection()
{
	IndicesToLayers.Reset();
	for (auto& Layer : Layers)
	{
		IndicesToLayers.Add(Layer.Index, Layer);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelLandscapeMaterialCollection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
	{
		return;
	}

	if (!Material)
	{
		Layers.Empty();
		MaterialCache.Empty();
		return;
	}
	
	CleanupCache();

	if (NeedsToBeConvertedToVoxel())
	{
		FVoxelMessages::FNotification Notification;
		Notification.UniqueId = OBJECT_LINE_ID();
		Notification.Message = FString::Printf(TEXT("%s is a landscape only material: it needs to be converted to work with both voxel and landscapes"), *Material->GetName());
		
		auto& Button = Notification.Buttons.Emplace_GetRef();
		Button.Text = "Fix Now";
		Button.Tooltip = "Fix the material";
		Button.OnClick = FSimpleDelegate::CreateWeakLambda(Material.Get(), [Material = Material, This = MakeWeakObjectPtr(this)]()
		{
			FVoxelEditorDelegates::FixVoxelLandscapeMaterial.Broadcast(Material->GetMaterial());

			if (This.IsValid())
			{
				This->PostEditChange();
				This->InitializeCollection();
			}
		});

		FVoxelMessages::ShowNotification(Notification);
		return;
	}

	FixupLayers();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UMaterialInstanceConstant* UVoxelLandscapeMaterialCollection::FindOrAddPermutation(const FVoxelLandscapeMaterialCollectionPermutation& Permutation) const
{
	UMaterialInstanceConstant* CachedMaterial = MaterialCache.FindRef(Permutation);
	if (CachedMaterial && CachedMaterial->Parent == Material)
	{
		return CachedMaterial;
	}

#if WITH_EDITOR
	return CreateInstanceForPermutation(Permutation);
#else
	return nullptr;
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
UMaterialInstanceConstant* UVoxelLandscapeMaterialCollection::CreateInstanceForPermutation(const FVoxelLandscapeMaterialCollectionPermutation& Permutation) const
{
	if (!GIsEditor)
	{
		// Standalone
		return nullptr;
	}
	if (!ensure(Material))
	{
		return nullptr;
	}
	if (NeedsToBeConvertedToVoxel())
	{
		return nullptr;
	}

	// Make sure the static permutations are always correct
	const_cast<UVoxelLandscapeMaterialCollection*>(this)->FixupLayers();
	
	FlushRenderingCommands();

	UMaterialInstanceConstant* Instance = NewObject<UMaterialInstanceConstant>(GetOuter());

	LOG_VOXEL(Log, TEXT("Looking for key '%s', making new combination %s"), *Permutation.ToString(), *Instance->GetName());
	MaterialCache.Add(Permutation, Instance);
	// Make sure we're saving the new instance
	MarkPackageDirty();

	Instance->SetParentEditorOnly(Material, false);
	
	FStaticParameterSet StaticParameters;
	{
		TSet<FName> AddedLayers;
		for (int32 Index = 0; Index < 6; Index++)
		{
			const auto Name = Permutation.Names[Index];
			if (Name.IsNone())
			{
				continue;
			}
			AddedLayers.Add(Name);

			FStaticTerrainLayerWeightParameter Parameter;
			Parameter.LayerName = Name;
			// Pass the layer to use to the voxel expression
			// Add 1 000 000 to detect voxel vs landscape indices
			Parameter.WeightmapIndex = 1000000 + Index;
			StaticParameters.EditorOnly.TerrainLayerWeightParameters.Add(Parameter);
		}

		for (auto& Layer : Layers)
		{
			if (AddedLayers.Contains(Layer.Name))
			{
				continue;
			}
			
			FStaticTerrainLayerWeightParameter Parameter;
			Parameter.LayerName = Layer.Name;
			// 1 000 006 is used to set Default
			Parameter.WeightmapIndex = 1000006;
			StaticParameters.EditorOnly.TerrainLayerWeightParameters.Add(Parameter);
		}
	}
	
	Instance->UpdateStaticPermutation(StaticParameters);
	Instance->PostEditChange();
	
	return Instance;
}

void UVoxelLandscapeMaterialCollection::ForeachMaterialParameterName(TFunctionRef<void(FName)> Lambda) const
{
	if (!ensure(Material))
	{
		return;
	}
	
	UMaterial* ActualMaterial = Material->GetMaterial();
	if (!ensure(ActualMaterial))
	{
		return;
	}

	if (!ensure(!NeedsToBeConvertedToVoxel()))
	{
		return;
	}

	TSet<FName> Names;
	for (const TObjectPtr<UMaterialExpression>& Expression : ActualMaterial->GetExpressions())
	{
		if (const UMaterialExpressionLandscapeLayerWeight* Weight = Cast<UMaterialExpressionLandscapeLayerWeight>(Expression))
		{
			Names.Add(Weight->ParameterName);
		}
		if (const UMaterialExpressionLandscapeLayerSwitch* Switch = Cast<UMaterialExpressionLandscapeLayerSwitch>(Expression))
		{
			Names.Add(Switch->ParameterName);
		}
		if (const UMaterialExpressionLandscapeLayerSample* Sample = Cast<UMaterialExpressionLandscapeLayerSample>(Expression))
		{
			Names.Add(Sample->ParameterName);
		}
		if (const UMaterialExpressionLandscapeLayerBlend* Blend = Cast<UMaterialExpressionLandscapeLayerBlend>(Expression))
		{
			for (const FLayerBlendInput& Layer : Blend->Layers)
			{
				Names.Add(Layer.LayerName);
			}
		}

		// Note: don't check landscape visibility parameter name, it just returns __LANDSCAPE_VISIBILITY__
	}

	// Make sure names are unique before iterating
	for (const FName Name : Names)
	{
		Lambda(Name);
	}
}

bool UVoxelLandscapeMaterialCollection::NeedsToBeConvertedToVoxel() const
{
	if (!ensure(Material))
	{
		return false;
	}
	
	UMaterial* ActualMaterial = Material->GetMaterial();
	if (!ensure(ActualMaterial))
	{
		return false;
	}

	return FVoxelMaterialExpressionUtilities::NeedsToBeConvertedToVoxel(ActualMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions);
}

void UVoxelLandscapeMaterialCollection::FixupLayers()
{
	VOXEL_FUNCTION_COUNTER();
	
	TSet<uint8> UsedIndices;
	TMap<FName, uint8> ExistingIndices;

	const auto GetUniqueIndex = [&](uint8 Index)
	{
		bool bIsAlreadyInSet = true;
		while (bIsAlreadyInSet)
		{
			UsedIndices.Add(Index, &bIsAlreadyInSet);
			if (bIsAlreadyInSet) Index++;
		}
		return Index;
	};
	
	for (auto& Layer : Layers)
	{
		ExistingIndices.Add(Layer.Name, GetUniqueIndex(Layer.Index));
	}

	Layers.Reset();
	ForeachMaterialParameterName([&](FName Name)
	{
		auto* ExistingIndex = ExistingIndices.Find(Name);
		const uint8 Index = ExistingIndex ? *ExistingIndex : GetUniqueIndex(0);
		
		Layers.Add(FVoxelLandscapeMaterialCollectionLayer{ Name, Index });
	});
}

void UVoxelLandscapeMaterialCollection::CleanupCache() const
{
	VOXEL_FUNCTION_COUNTER();
	
	for (auto It = MaterialCache.CreateIterator(); It; ++It)
	{
		if (It.Value() && It.Value()->Parent != Material)
		{
			It.RemoveCurrent();
		}
	}
}
#endif