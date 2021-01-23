// Copyright 2021 Phyronnaz

#include "VoxelFoliageCustomDataCustomization.h"
#include "VoxelFoliageCustomData.h"
#include "VoxelEditorDetailsIncludes.h"

void FVoxelFoliageCustomDataCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	void* Address = nullptr;
	if (!ensure(PropertyHandle->GetValueData(Address) == FPropertyAccess::Success) || !ensure(Address))
	{
		return;
	}
	FVoxelFoliageCustomData& CustomData = *static_cast<FVoxelFoliageCustomData*>(Address);
	
	GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageCustomData, Type)->SetOnPropertyValueChanged(FVoxelEditorUtilities::MakeRefreshDelegate(CustomizationUtils));
	GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageCustomData, bUseMainGenerator)->SetOnPropertyValueChanged(FVoxelEditorUtilities::MakeRefreshDelegate(CustomizationUtils));

	IDetailGroup* Group = &ChildBuilder.AddGroup(TEXT("Foliage Custom Data Type"), PropertyHandle->GetPropertyDisplayName());
	Group->HeaderRow()
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageCustomData, Type)->CreatePropertyValueWidget()
	];

	switch (CustomData.Type)
	{
	case EVoxelFoliageCustomDataType::ColorGeneratorOutput:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageCustomData, bUseMainGenerator));
		if (!CustomData.bUseMainGenerator)
		{
			Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageCustomData, CustomGenerator));
		}
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageCustomData, ColorGeneratorOutputName));
		break;
	}
	case EVoxelFoliageCustomDataType::FloatGeneratorOutput:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageCustomData, bUseMainGenerator));
		if (!CustomData.bUseMainGenerator)
		{
			Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageCustomData, CustomGenerator));
		}
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageCustomData, FloatGeneratorOutputName));
		break;
	}
	case EVoxelFoliageCustomDataType::MaterialColor:
	{
		break;
	}
	case EVoxelFoliageCustomDataType::MaterialSingleIndex:
	{
		break;
	}
	case EVoxelFoliageCustomDataType::MaterialUV:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageCustomData, UVChannel));
		Group->AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageCustomData, UVAxis));
		break;
	}
	default: ensure(false);
	}
}