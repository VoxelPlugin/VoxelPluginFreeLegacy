// Copyright 2020 Phyronnaz

#include "Details/BoolVectorCustomization.h"
#include "VoxelGlobals.h"
#include "BoolVector.h"

#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "Voxel"

#define GET_CHILD_PROPERTY(PropertyHandle, Class, Property) PropertyHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(Class, Property)).ToSharedRef()

void FBoolVectorCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	const float XYZPadding = 5.0f;

	const auto X = GET_CHILD_PROPERTY(PropertyHandle, FBoolVector, bX);
	const auto Y = GET_CHILD_PROPERTY(PropertyHandle, FBoolVector, bY);
	const auto Z = GET_CHILD_PROPERTY(PropertyHandle, FBoolVector, bZ);
	
	HeaderRow
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("LockPositionLabel", "Lock Position"))
		.ToolTipText(LOCTEXT("LockPositionTooltip", "Locks movement along the specified axis"))
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

void FBoolVectorCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	
}

#undef GET_CHILD_PROPERTY
#undef LOCTEXT_NAMESPACE