// Copyright 2020 Phyronnaz

#include "VoxelRender/MaterialCollections/VoxelLandscapeMaterialCollection.h"
#include "VoxelRender/VoxelMaterialIndices.h"
#include "VoxelRender/VoxelMaterialExpressions.h"
#include "VoxelMessages.h"
#include "VoxelEditorDelegates.h"
#include "VoxelFeedbackContext.h"

#include "Misc/MessageDialog.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"

UMaterialInterface* UVoxelLandscapeMaterialCollection::GetVoxelMaterial(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const
{
	if (!Material)
	{
		return nullptr;
	}

	return FindOrAddPermutation(MakePermutation(Indices));
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
		Button.OnClick = FSimpleDelegate::CreateWeakLambda(Material, [Material = Material, This = MakeWeakObjectPtr(this)]()
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

#if WITH_EDITOR
void UVoxelLandscapeMaterialCollection::BuildAllPermutations()
{
	CleanupCache();
	InitializeCollection();
	
	TArray<int32> LayerIndices;
	for (auto& Layer : Layers)
	{
		LayerIndices.AddUnique(Layer.Index);
	}
	LayerIndices.Sort();
	
	TArray<FVoxelLandscapeMaterialCollectionPermutation> PermutationsToBuild;
	for (int32 NumIndices = 1; NumIndices <= MaxMaterialsToBlendAtOnce; NumIndices++)
	{
		FVoxelMaterialIndices Indices;
		Indices.NumIndices = NumIndices;

		const auto Add = [&]()
		{
			for (int32 Index = 1; Index < NumIndices; Index++)
			{
				ensure(Indices.SortedIndices[Index - 1] < Indices.SortedIndices[Index]);
			}

			const auto Permutation = MakePermutation(Indices);
			if (!MaterialCache.Contains(Permutation))
			{
				PermutationsToBuild.Add(Permutation);
			}
		};

		for (int32 Index0 = 0; Index0 < LayerIndices.Num(); Index0++)
		{
			Indices.SortedIndices[0] = LayerIndices[Index0];
			if (NumIndices == 1) 
			{
				Add();
				continue;
			}

			for (int32 Index1 = Index0 + 1; Index1 < LayerIndices.Num(); Index1++)
			{
				Indices.SortedIndices[1] = LayerIndices[Index1];
				if (NumIndices == 2)
				{
					Add();
					continue;
				}

				for (int32 Index2 = Index1 + 1; Index2 < LayerIndices.Num(); Index2++)
				{
					Indices.SortedIndices[2] = LayerIndices[Index2];
					if (NumIndices == 3)
					{
						Add();
						continue;
					}

					for (int32 Index3 = Index2 + 1; Index3 < LayerIndices.Num(); Index3++)
					{
						Indices.SortedIndices[3] = LayerIndices[Index3];
						if (NumIndices == 4)
						{
							Add();
							continue;
						}

						for (int32 Index4 = Index3 + 1; Index4 < LayerIndices.Num(); Index4++)
						{
							Indices.SortedIndices[4] = LayerIndices[Index4];
							if (NumIndices == 5)
							{
								Add();
								continue;
							}

							for (int32 Index5 = Index4 + 1; Index5 < LayerIndices.Num(); Index5++)
							{
								Indices.SortedIndices[5] = LayerIndices[Index5];
								Add();
							}
						}
					}
				}
			}
		}
	}

	if (PermutationsToBuild.Num() == 0)
	{
		FVoxelMessages::ShowNotification("No permutation to build");
		return;
	}
	
	const EAppReturnType::Type Result = FMessageDialog::Open(
		EAppMsgType::YesNo,
		FText::Format(VOXEL_LOCTEXT("This will create {0} new materials. Do you want to continue?\n\n To reduce permutations, decrease MaxMaterialsToBlendAtOnce or add layers to LayersToIgnore"), PermutationsToBuild.Num()));

	if (Result == EAppReturnType::No)
	{
		return;
	}

	FVoxelScopedSlowTask SlowTask(PermutationsToBuild.Num(), VOXEL_LOCTEXT("Creating materials"));
	SlowTask.MakeDialog(true);

	for (auto& Permutation : PermutationsToBuild)
	{
		SlowTask.EnterProgressFrame();
		if (SlowTask.ShouldCancel())
		{
			break;
		}
		
		FindOrAddPermutation(Permutation);
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelLandscapeMaterialCollectionPermutation UVoxelLandscapeMaterialCollection::MakePermutation(const FVoxelMaterialIndices& Indices) const
{
	FVoxelLandscapeMaterialCollectionPermutation Permutation;
	for (int32 Index = 0; Index < Indices.NumIndices; Index++)
	{
		Permutation.Names[Index] = IndicesToLayers.FindRef(Indices.SortedIndices[Index]).Name;
	}
	return Permutation;
}

UMaterialInstanceConstant* UVoxelLandscapeMaterialCollection::FindOrAddPermutation(const FVoxelLandscapeMaterialCollectionPermutation& Permutation) const
{
	auto* CachedMaterial = MaterialCache.FindRef(Permutation);
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
		struct FInfoAndGuid
		{
			FMaterialParameterInfo Info;
			FGuid Guid;
		};
		TMap<FName, FInfoAndGuid> NameToInfo;
		ForeachMaterialParameter([&](const FMaterialParameterInfo& Info, const FGuid& Guid)
		{
			ensure(!NameToInfo.Contains(Info.Name));
			NameToInfo.Add(Info.Name, { Info, Guid });
		});

		TSet<FName> AddedLayers;
		for (int32 Index = 0; Index < 6; Index++)
		{
			const auto Name = Permutation.Names[Index];
			if (Name.IsNone() || !ensure(NameToInfo.Contains(Name)))
			{
				continue;
			}
			AddedLayers.Add(Name);

			FStaticTerrainLayerWeightParameter Parameter;
			Parameter.ParameterInfo = NameToInfo[Name].Info;
			Parameter.bOverride = true;
			Parameter.ExpressionGUID = NameToInfo[Name].Guid;
			// Pass the layer to use to the voxel expression
			// Add 1 000 000 to detect voxel vs landscape indices
			Parameter.WeightmapIndex = 1000000 + Index;
			StaticParameters.TerrainLayerWeightParameters.Add(Parameter);
		}

		for (auto& Layer : Layers)
		{
			if (AddedLayers.Contains(Layer.Name))
			{
				continue;
			}
			if (!ensure(NameToInfo.Contains(Layer.Name)))
			{
				continue;
			}
			
			FStaticTerrainLayerWeightParameter Parameter;
			Parameter.ParameterInfo = NameToInfo[Layer.Name].Info;
			Parameter.bOverride = true;
			Parameter.ExpressionGUID = NameToInfo[Layer.Name].Guid;
			// 1 000 006 is used to set Default
			Parameter.WeightmapIndex = 1000006;
			StaticParameters.TerrainLayerWeightParameters.Add(Parameter);
		}
	}
	
	Instance->UpdateStaticPermutation(StaticParameters);
	Instance->PostEditChange();
	
	return Instance;
}

void UVoxelLandscapeMaterialCollection::ForeachMaterialParameter(TFunctionRef<void(const FMaterialParameterInfo&, const FGuid&)> Lambda) const
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

	TArray<FMaterialParameterInfo> ParameterInfos;
	TArray<FGuid> Guids;

	ActualMaterial->GetAllParameterInfo<UMaterialExpressionLandscapeLayerWeight>(ParameterInfos, Guids);
	ActualMaterial->GetAllParameterInfo<UMaterialExpressionLandscapeLayerSwitch>(ParameterInfos, Guids);
	ActualMaterial->GetAllParameterInfo<UMaterialExpressionLandscapeLayerSample>(ParameterInfos, Guids);
	ActualMaterial->GetAllParameterInfo<UMaterialExpressionLandscapeLayerBlend>(ParameterInfos, Guids);
	// Note: don't query landscape visibility parameter name, it just returns __LANDSCAPE_VISIBILITY__
	
	if (!ensure(ParameterInfos.Num() == Guids.Num()))
	{
		return;
	}
	
	for (int32 Index = 0; Index < ParameterInfos.Num(); Index++)
	{
		Lambda(ParameterInfos[Index], Guids[Index]);
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

	return FVoxelMaterialExpressionUtilities::NeedsToBeConvertedToVoxel(ActualMaterial->Expressions);
}

void UVoxelLandscapeMaterialCollection::FixupLayers()
{
	VOXEL_FUNCTION_COUNTER();

	// LayersToIgnore
	{
		const TMap<FName, bool> CurrentLayersToIgnore = MoveTemp(LayersToIgnore);
		ForeachMaterialParameter([&](const FMaterialParameterInfo& Info, const FGuid& Guid)
		{
			LayersToIgnore.Add(Info.Name, CurrentLayersToIgnore.FindRef(Info.Name));
		});
	}
		
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
	ForeachMaterialParameter([&](const FMaterialParameterInfo& Info, const FGuid& Guid)
	{
		if (LayersToIgnore.FindRef(Info.Name))
		{
			return;
		}
		
		auto* ExistingIndex = ExistingIndices.Find(Info.Name);
		const uint8 Index = ExistingIndex ? *ExistingIndex : GetUniqueIndex(0);
		
		Layers.Add(FVoxelLandscapeMaterialCollectionLayer{ Info.Name, Index });
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