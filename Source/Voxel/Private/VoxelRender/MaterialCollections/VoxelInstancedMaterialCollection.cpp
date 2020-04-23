// Copyright 2020 Phyronnaz

#include "VoxelRender/MaterialCollections/VoxelInstancedMaterialCollection.h"
#include "VoxelMaterialUtilities.h"
#include "VoxelUniqueError.h"
#include "VoxelMessages.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstanceConstant.h"

int32 UVoxelInstancedMaterialCollection::GetMaxMaterialIndices() const
{
	if (MaxMaterialsToBlendAtOnce < 1 || MaxMaterialsToBlendAtOnce > 6)
	{
		FVoxelMessages::Error<EVoxelShowNotification::Hide>("MaxMaterialsToBlendAtOnce should be between 1 and 6", this);
		return 1;
	}

	const auto CheckTemplates = [&](const FVoxelInstancedMaterialCollectionTemplates& Templates, bool bTessellated)
	{
		const FString& TessellatedString = bTessellated ? "Tessellated " : "";
		if (!Templates.Template1x)
		{
			FVoxelMessages::Error<EVoxelShowNotification::Hide>("Missing " + TessellatedString + "Template 1x!", this);
		}
		if (MaxMaterialsToBlendAtOnce > 1 && !Templates.Template2x)
		{
			FVoxelMessages::Error<EVoxelShowNotification::Hide>(TessellatedString + "Template 2x is null!", this);
		}
		if (MaxMaterialsToBlendAtOnce > 2 && !Templates.Template3x)
		{
			FVoxelMessages::Error<EVoxelShowNotification::Hide>(TessellatedString + "Template 3x is null!", this);
		}
		if (MaxMaterialsToBlendAtOnce > 3 && !Templates.Template4x)
		{
			FVoxelMessages::Error<EVoxelShowNotification::Hide>(TessellatedString + "Template 4x is null!", this);
		}
		if (MaxMaterialsToBlendAtOnce > 4 && !Templates.Template5x)
		{
			FVoxelMessages::Error<EVoxelShowNotification::Hide>(TessellatedString + "Template 5x is null!", this);
		}
		if (MaxMaterialsToBlendAtOnce > 5 && !Templates.Template6x)
		{
			FVoxelMessages::Error<EVoxelShowNotification::Hide>(TessellatedString + "Template 6x is null!", this);
		}

		const FString TessellatedError = bTessellated
			? " should have tessellation enabled!"
			: " should have tessellation disabled! Use the tessellation materials instead";
		const auto Check = [&](auto* Material, const FString& Name)
		{
			if (Material && FVoxelUtilities::IsMaterialTessellated(Material) != bTessellated)
			{
				FVoxelMessages::Error<EVoxelShowNotification::Hide>(TessellatedString + Name + TessellatedError, this);
			}
		};

		Check(Templates.Template1x, "Template 1x");
		Check(Templates.Template2x, "Template 2x");
		Check(Templates.Template3x, "Template 3x");
		Check(Templates.Template4x, "Template 4x");
		Check(Templates.Template5x, "Template 5x");
		Check(Templates.Template6x, "Template 6x");
	};

	CheckTemplates(NormalTemplates, false);
	if (bEnableTessellation)
	{
		CheckTemplates(TessellatedTemplates, true);
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

UMaterialInterface* UVoxelInstancedMaterialCollection::GetVoxelMaterial_NotCached(const FVoxelMaterialIndices& Indices, bool bTessellation, uint64 UniqueIdForErrors) const
{
	if (bTessellation && !bEnableTessellation)
	{
		static TVoxelUniqueError<> UniqueError;
		FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
			UniqueError(UniqueIdForErrors, {}),
			"Voxel World has tessellation enabled, but Enable Tessellation = false on this collection!",
			this);
		return nullptr;
	}
	
	// Will happen if the material collection is changed at runtime to a one with a different MaxMaterialsToBlendAtOnce
	if (!ensure(Indices.NumIndices <= MaxMaterialsToBlendAtOnce) || !ensure(MaxMaterialsToBlendAtOnce > 0)) return nullptr;
	
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

	const auto GetTemplate = [&](const FVoxelInstancedMaterialCollectionTemplates& Templates)
	{
		switch (Indices.NumIndices)
		{
		default: ensure(false);
		case 1: return Templates.Template1x;
		case 2: return Templates.Template2x;
		case 3: return Templates.Template3x;
		case 4: return Templates.Template4x;
		case 5: return Templates.Template5x;
		}
	};
	
	UMaterialInterface* const Template = bTessellation ? GetTemplate(TessellatedTemplates) : GetTemplate(NormalTemplates);
	if (!Template) return nullptr;

	auto* DynamicInstance = UMaterialInstanceDynamic::Create(Template, nullptr);
	if (!ensure(DynamicInstance)) return nullptr;

	const auto SetParameters = [&](UMaterialInstance* Instance, const FString& Prefix)
	{
		if (!Instance->GetStaticParameters().IsEmpty())
		{
			static TVoxelUniqueError<TWeakObjectPtr<UMaterialInstance>> UniqueError;
			FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
				UniqueError(UniqueIdForErrors, Instance),
				FString::Printf(TEXT("%s has static parameters overrides, this is not supported by instanced collections!"), *Instance->GetName()),
				this);
		}

		const auto GetNewName = [&](FName Name)
		{
			return FName(*(Prefix + Name.ToString()));
		};

		for (const auto& Parameter : Instance->ScalarParameterValues)
		{
			DynamicInstance->SetScalarParameterValue(GetNewName(Parameter.ParameterInfo.Name), Parameter.ParameterValue);
		}
		for (const auto& Parameter : Instance->VectorParameterValues)
		{
			DynamicInstance->SetVectorParameterValue(GetNewName(Parameter.ParameterInfo.Name), Parameter.ParameterValue);
		}
		for (const auto& Parameter : Instance->TextureParameterValues)
		{
			DynamicInstance->SetTextureParameterValue(GetNewName(Parameter.ParameterInfo.Name), Parameter.ParameterValue);
		}
		for (const auto& Parameter : Instance->FontParameterValues)
		{
			DynamicInstance->SetFontParameterValue(GetNewName(Parameter.ParameterInfo.Name), Parameter.FontValue, Parameter.FontPage);
		}
	};

	for (int32 Index = 0; Index < Indices.NumIndices; Index++)
	{
		auto* Instance = Layers.FindByKey(Indices.SortedIndices[Index])->LayerMaterialInstance;
		SetParameters(Instance, FString::Printf(TEXT("VOXELPARAM_%d:"), Index));
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