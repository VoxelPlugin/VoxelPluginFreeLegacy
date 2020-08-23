// Copyright 2020 Phyronnaz

#include "Details/VoxelBoolVectorCustomization.h"
#include "VoxelMinimal.h"
#include "VoxelBoolVector.h"

#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Text/STextBlock.h"

#define GET_CHILD_PROPERTY(PropertyHandle, Class, Property) PropertyHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(Class, Property)).ToSharedRef()

void FVoxelBoolVectorCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	const float XYZPadding = 5.0f;

	const auto X = GET_CHILD_PROPERTY(PropertyHandle, FVoxelBoolVector, bX);
	const auto Y = GET_CHILD_PROPERTY(PropertyHandle, FVoxelBoolVector, bY);
	const auto Z = GET_CHILD_PROPERTY(PropertyHandle, FVoxelBoolVector, bZ);
	
	HeaderRow
	.NameContent()
	[
		SNew(STextBlock)
		.Text(VOXEL_LOCTEXT("Lock Position"))
		.ToolTipText(VOXEL_LOCTEXT("Locks movement along the specified axis"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(0.f, 0.f, XYZPadding, 0.f)
		.AutoWidth()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				X->CreatePropertyNameWidget()
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				X->CreatePropertyValueWidget()
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(0.f, 0.f, XYZPadding, 0.f)
		.AutoWidth()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				Y->CreatePropertyNameWidget()
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				Y->CreatePropertyValueWidget()
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(0.f, 0.f, XYZPadding, 0.f)
		.AutoWidth()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				Z->CreatePropertyNameWidget()
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				Z->CreatePropertyValueWidget()
			]
		]
	];
}

void FVoxelBoolVectorCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	
}

#undef GET_CHILD_PROPERTY