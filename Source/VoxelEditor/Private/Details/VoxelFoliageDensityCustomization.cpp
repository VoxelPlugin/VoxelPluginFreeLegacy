// Copyright 2020 Phyronnaz

#include "Details/VoxelFoliageDensityCustomization.h"
#include "VoxelSpawners/VoxelFoliageDensity.h"
#include "VoxelEditorDetailsIncludes.h"

void FVoxelFoliageDensityCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	void* Address = nullptr;
	if (!ensure(PropertyHandle->GetValueData(Address) == FPropertyAccess::Success) || !ensure(Address))
	{
		return;
	}
	FVoxelFoliageDensity& Density = *static_cast<FVoxelFoliageDensity*>(Address);
	
	GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, Type)->SetOnPropertyValueChanged(FVoxelEditorUtilities::MakeRefreshDelegate(CustomizationUtils));
	GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, bUseMainGenerator)->SetOnPropertyValueChanged(FVoxelEditorUtilities::MakeRefreshDelegate(CustomizationUtils));

	IDetailGroup* Group = &ChildBuilder.AddGroup(TEXT("Spawner Density Type"), PropertyHandle->GetPropertyDisplayName());
	Group->HeaderRow()
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, Type)->CreatePropertyValueWidget()
	];

	switch (Density.Type)
	{
	default: ensure(false);
	case EVoxelFoliageDensityType::Constant:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, Constant));
		break;
	}
	case EVoxelFoliageDensityType::GeneratorOutput:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, bUseMainGenerator));
		if (!Density.bUseMainGenerator)
		{
			Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, CustomGenerator));
		}
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, GeneratorOutputName));
		break;
	}
	case EVoxelFoliageDensityType::MaterialRGBA:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, RGBAChannel));
		break;
	}
	case EVoxelFoliageDensityType::MaterialUVs:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, UVChannel));
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, UVAxis));
		break;
	}
	case EVoxelFoliageDensityType::MaterialFiveWayBlend:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, FiveWayBlendChannel));
		break;
	}
	case EVoxelFoliageDensityType::SingleIndex:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, SingleIndexChannels));
		break;
	}
	case EVoxelFoliageDensityType::MultiIndex:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, MultiIndexChannels));
		break;
	}
	}
	Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageDensity, bInvertDensity));
}