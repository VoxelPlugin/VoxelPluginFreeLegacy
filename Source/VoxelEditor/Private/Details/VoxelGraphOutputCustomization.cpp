// Copyright 2021 Phyronnaz

#include "Details/VoxelGraphOutputCustomization.h"
#include "VoxelGraphOutputs.h"
#include "VoxelEditorDetailsIncludes.h"

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