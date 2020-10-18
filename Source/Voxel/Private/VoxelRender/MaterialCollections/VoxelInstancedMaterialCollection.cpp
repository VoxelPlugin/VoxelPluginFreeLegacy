// Copyright 2020 Phyronnaz

#include "VoxelRender/MaterialCollections/VoxelInstancedMaterialCollection.h"
#include "VoxelUtilities/VoxelMaterialUtilities.h"
#include "VoxelUniqueError.h"
#include "VoxelMessages.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstanceConstant.h"

#if WITH_EDITOR
void UVoxelInstancedMaterialCollectionTemplates::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty)
	{
		if (Template)
		{
			const auto CreateTemplate = [&](int32 Num)
			{
				auto* MaterialInstance = NewObject<UMaterialInstanceConstant>(this);
				MaterialInstance->SetParentEditorOnly(Template);
				
				FStaticParameterSet StaticParameters;
				MaterialInstance->GetStaticParameterValues(StaticParameters);

				const auto SetStaticParameter = [&](FName Name, bool bValue)
				{
					bool bFound = false;
					for (auto& StaticSwitchParameter : StaticParameters.StaticSwitchParameters)
					{
						if (StaticSwitchParameter.ParameterInfo.Name == Name)
						{
							// Note: we allow duplicates
							bFound = true;
							StaticSwitchParameter.bOverride = true;
							StaticSwitchParameter.Value = bValue;
						}
					}
					if (!bFound)
					{
						FVoxelMessages::Warning("Static parameter " + Name.ToString() + " not found in " + Template->GetName(), this);
					}
				};

				SetStaticParameter("Num Blends > 1", Num > 1);
				SetStaticParameter("Num Blends > 2", Num > 2);
				SetStaticParameter("Num Blends > 3", Num > 3);
				SetStaticParameter("Num Blends > 4", Num > 4);
				SetStaticParameter("Num Blends > 5", Num > 5);
				
				MaterialInstance->UpdateStaticPermutation(StaticParameters);
				MaterialInstance->PostEditChange();
				
				return MaterialInstance;
			};
			
			Template1x = CreateTemplate(1);
			Template2x = CreateTemplate(2);
			Template3x = CreateTemplate(3);
			Template4x = CreateTemplate(4);
			Template5x = CreateTemplate(5);
			Template6x = CreateTemplate(6);
		}
		else
		{
			Template1x = nullptr;
			Template2x = nullptr;
			Template3x = nullptr;
			Template4x = nullptr;
			Template5x = nullptr;
			Template6x = nullptr;
		}
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////

UVoxelInstancedMaterialCollection::UVoxelInstancedMaterialCollection()
{
	Redirects.Add(" Sides" );
	Redirects.Add(" Bottom" );
}

int32 UVoxelInstancedMaterialCollection::GetMaxMaterialIndices() const
{
	if (MaxMaterialsToBlendAtOnce < 1 || MaxMaterialsToBlendAtOnce > 6)
	{
		FVoxelMessages::Error<EVoxelShowNotification::Hide>("MaxMaterialsToBlendAtOnce should be between 1 and 6", this);
		return 1;
	}
	
	if (!Templates)
	{
		FVoxelMessages::Error<EVoxelShowNotification::Hide>("Missing Templates!", this);
		return 1;
	}
	
	if (!Templates->Template1x)
	{
		FVoxelMessages::Error<EVoxelShowNotification::Hide>("Template 1x is null!", this);
	}
	if (MaxMaterialsToBlendAtOnce > 1 && !Templates->Template2x)
	{
		FVoxelMessages::Error<EVoxelShowNotification::Hide>("Template 2x is null!", this);
	}
	if (MaxMaterialsToBlendAtOnce > 2 && !Templates->Template3x)
	{
		FVoxelMessages::Error<EVoxelShowNotification::Hide>("Template 3x is null!", this);
	}
	if (MaxMaterialsToBlendAtOnce > 3 && !Templates->Template4x)
	{
		FVoxelMessages::Error<EVoxelShowNotification::Hide>("Template 4x is null!", this);
	}
	if (MaxMaterialsToBlendAtOnce > 4 && !Templates->Template5x)
	{
		FVoxelMessages::Error<EVoxelShowNotification::Hide>("Template 5x is null!", this);
	}
	if (MaxMaterialsToBlendAtOnce > 5 && !Templates->Template6x)
	{
		FVoxelMessages::Error<EVoxelShowNotification::Hide>("Template 6x is null!", this);
	}

	TSet<int32> Indices;
	for (auto& Layer : Layers)
	{
		bool bIsAlreadyInSet;
		Indices.Add(Layer.LayerIndex, &bIsAlreadyInSet);

		if (bIsAlreadyInSet)
		{
			FVoxelMessages::Error<EVoxelShowNotification::Hide>(FString::Printf(TEXT("Index %d is used multiple times!"), Layer.LayerIndex), this);
		}
	}

	return MaxMaterialsToBlendAtOnce;
}

int32 UVoxelInstancedMaterialCollection::GetMaterialIndex(FName Name) const
{
	for (auto& Layer : Layers)
	{
		if (Layer.LayerMaterialInstance && Layer.LayerMaterialInstance->GetFName() == Name)
		{
			return Layer.LayerIndex;
		}
	}
	return -1;
}

TArray<UVoxelMaterialCollectionBase::FMaterialInfo> UVoxelInstancedMaterialCollection::GetMaterials() const
{
	TArray<FMaterialInfo> Result;
	for (const auto& Layer : Layers)
	{
		Result.Add(FMaterialInfo{ Layer.LayerIndex, FName(), Layer.LayerMaterialInstance });
	}
	return Result;
}

UMaterialInterface* UVoxelInstancedMaterialCollection::GetIndexMaterial(uint8 Index) const
{
	for (const auto& Layer : Layers)
	{
		if (Layer.LayerIndex == Index)
		{
			return Layer.LayerMaterialInstance;
		}
	}
	return nullptr;
}

UMaterialInterface* UVoxelInstancedMaterialCollection::GetVoxelMaterial_NotCached(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const
{
	VOXEL_FUNCTION_COUNTER();
	
	// Will happen if the material collection is changed at runtime to a one with a different MaxMaterialsToBlendAtOnce
	if (Indices.NumIndices > MaxMaterialsToBlendAtOnce || !ensure(MaxMaterialsToBlendAtOnce > 0)) 
	{
		return nullptr;
	}
	
	for (int32 Index = 0; Index < Indices.NumIndices; Index++)
	{
		const int32 SortedIndex = Indices.SortedIndices[Index];
		auto* Layer = Layers.FindByKey(SortedIndex);
		if (!Layer || !Layer->LayerMaterialInstance)
		{
			static TVoxelUniqueError<uint8> UniqueError;
			FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
				UniqueError(UniqueIdForErrors, Index),
				FString::Printf(TEXT("No layer with index %d, or layer has no material instance!"), SortedIndex),
				this);
			return nullptr;
		}
	}

	if (!Templates)
	{
		return nullptr;
	}

	const auto GetTemplate = [&]()
	{
		switch (Indices.NumIndices)
		{
		default: ensure(false);
		case 1: return Templates->Template1x;
		case 2: return Templates->Template2x;
		case 3: return Templates->Template3x;
		case 4: return Templates->Template4x;
		case 5: return Templates->Template5x;
		case 6: return Templates->Template6x;
		}
	};
	
	UMaterialInterface* const Template = GetTemplate();
	if (!Template)
	{
		return nullptr;
	}
	
	auto* DynamicInstance = UMaterialInstanceDynamic::Create(Template, nullptr);
	if (!ensure(DynamicInstance)) return nullptr;

	const auto SetParameters = [&](UMaterialInstance* Instance, const FString& Prefix)
	{
		const auto GetNewName = [&](FName Name, const FString& Suffix)
		{
			return FName(*(Prefix + Name.ToString() + Suffix));
		};

		TSet<FName> OverridenParameterNames;
		for (const auto& Parameter : Instance->ScalarParameterValues)
		{
			const FName Name = GetNewName(Parameter.ParameterInfo.Name, {});
			DynamicInstance->SetScalarParameterValue(Name, Parameter.ParameterValue);
			OverridenParameterNames.Add(Name);
		}
		for (const auto& Parameter : Instance->VectorParameterValues)
		{
			const FName Name = GetNewName(Parameter.ParameterInfo.Name, {});
			DynamicInstance->SetVectorParameterValue(Name, Parameter.ParameterValue);
			OverridenParameterNames.Add(Name);
		}
		for (const auto& Parameter : Instance->TextureParameterValues)
		{
			const FName Name = GetNewName(Parameter.ParameterInfo.Name, {});
			DynamicInstance->SetTextureParameterValue(Name, Parameter.ParameterValue);
			OverridenParameterNames.Add(Name);
		}
		for (const auto& Parameter : Instance->FontParameterValues)
		{
			const FName Name = GetNewName(Parameter.ParameterInfo.Name, {});
			DynamicInstance->SetFontParameterValue(Name, Parameter.FontValue, Parameter.FontPage);
			OverridenParameterNames.Add(Name);
		}

		for (const FString& Redirect : Redirects)
		{
			for (const auto& Parameter : Instance->ScalarParameterValues)
			{
				const FName Name = GetNewName(Parameter.ParameterInfo.Name, Redirect);
				if (!OverridenParameterNames.Contains(Name))
				{
					DynamicInstance->SetScalarParameterValue(Name, Parameter.ParameterValue);
				}
			}
			for (const auto& Parameter : Instance->VectorParameterValues)
			{
				const FName Name = GetNewName(Parameter.ParameterInfo.Name, Redirect);
				if (!OverridenParameterNames.Contains(Name))
				{
					DynamicInstance->SetVectorParameterValue(Name, Parameter.ParameterValue);
				}
			}
			for (const auto& Parameter : Instance->TextureParameterValues)
			{
				const FName Name = GetNewName(Parameter.ParameterInfo.Name, Redirect);
				if (!OverridenParameterNames.Contains(Name))
				{
					DynamicInstance->SetTextureParameterValue(Name, Parameter.ParameterValue);
				}
			}
			for (const auto& Parameter : Instance->FontParameterValues)
			{
				const FName Name = GetNewName(Parameter.ParameterInfo.Name, Redirect);
				if (!OverridenParameterNames.Contains(Name))
				{
					DynamicInstance->SetFontParameterValue(Name, Parameter.FontValue, Parameter.FontPage);
				}
			}
		}
	};

	for (int32 Index = 0; Index < Indices.NumIndices; Index++)
	{
		auto* Instance = Layers.FindByKey(Indices.SortedIndices[Index])->LayerMaterialInstance;
		SetParameters(Instance, ParametersPrefix + FString::Printf(TEXT("%d:"), Index));
	}

	return DynamicInstance;
}

#if WITH_EDITOR
void UVoxelInstancedMaterialCollection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelInstancedMaterialCollectionInstance::InitializeCollection()
{
	Super::InitializeCollection();

	if (LayersSource)
	{
		Layers = LayersSource->Layers;
	}
}

void UVoxelInstancedMaterialCollectionInstance::PostLoad()
{
	Super::PostLoad();

	if (LayersSource)
	{
		Layers = LayersSource->Layers;
	}
}

#if WITH_EDITOR
void UVoxelInstancedMaterialCollectionInstance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (LayersSource)
	{
		Layers = LayersSource->Layers;
	}
}

bool UVoxelInstancedMaterialCollectionInstance::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty && InProperty->GetFName() == GET_MEMBER_NAME_STATIC(UVoxelInstancedMaterialCollectionInstance, Layers))
	{
		return false;
	}

	return Super::CanEditChange(InProperty);
}
#endif