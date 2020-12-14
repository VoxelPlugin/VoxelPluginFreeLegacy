// Copyright 2020 Phyronnaz

#include "Details/VoxelDistanceCustomization.h"
#include "VoxelDistance.h"
#include "VoxelEditorDetailsIncludes.h"

void FVoxelDistanceCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	const auto TypeHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelDistance, Type);
	const auto DistanceHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelDistance, Distance);

	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MaxDesiredWidth(TOptional<float>())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(100.f)
			[
				DistanceHandle->CreatePropertyValueWidget()
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(5, 0, 0, 0)
		.AutoWidth()
		[
			TypeHandle->CreatePropertyValueWidget()
		]
	];
}
