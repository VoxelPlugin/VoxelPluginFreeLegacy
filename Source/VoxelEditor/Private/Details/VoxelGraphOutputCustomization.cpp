// Copyright 2020 Phyronnaz

#include "Details/VoxelGraphOutputCustomization.h"
#include "VoxelGraphOutputs.h"
#include "VoxelMinimal.h"

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "Widgets/Layout/SSpacer.h"

#define GET_CHILD_PROPERTY(PropertyHandle, Class, Property) PropertyHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(Class, Property)).ToSharedRef()

void FVoxelGraphOutputCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	auto NameHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelGraphOutput, Name);
	auto CategoryHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelGraphOutput, Category);

	HeaderRow
	.NameContent()
	.MinDesiredWidth(125.0f)
	.MaxDesiredWidth(125.0f)
	[
		NameHandle->CreatePropertyValueWidget()
	]
	.ValueContent()
	[
		CategoryHandle->CreatePropertyValueWidget()
	];
}

#undef GET_CHILD_PROPERTY