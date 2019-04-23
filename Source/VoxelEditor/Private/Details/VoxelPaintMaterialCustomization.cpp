// Copyright 2019 Phyronnaz

#include "Details/VoxelPaintMaterialCustomization.h"
#include "VoxelTools/VoxelPaintMaterial.h"

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "IDetailGroup.h"

#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "Voxel"

void FVoxelPaintMaterialCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

#define GET_CHILD_PROPERTY(PropertyHandle, Class, Property) PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(Class, Property)).ToSharedRef()

void FVoxelPaintMaterialCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	auto TypeHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, Type);

	FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateLambda([&CustomizationUtils]()
	{
		auto Utilities = CustomizationUtils.GetPropertyUtilities();
		if (Utilities.IsValid())
		{
			Utilities->ForceRefresh();
		}
	});
	TypeHandle->SetOnPropertyValueChanged(RefreshDelegate);
	
	IDetailGroup& Group = ChildBuilder.AddGroup( TEXT("Paint Material Type"), PropertyHandle->GetPropertyDisplayName() );
	Group.HeaderRow()
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		TypeHandle->CreatePropertyValueWidget()
	];
	
	FString Type;
	TypeHandle->GetValueAsFormattedString(Type);
	if (Type == "RGB")
	{
		auto ColorHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, Color);
		Group.AddPropertyRow(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, Color));
		Group.AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, Amount));
		Group.AddPropertyRow(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bPaintR));
		Group.AddPropertyRow(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bPaintG));
		Group.AddPropertyRow(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bPaintB));
		Group.AddPropertyRow(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bPaintA));
	}
	else if (Type == "SingleIndex")
	{
		Group.AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, Index));
	}
	else if (Type == "DoubleIndexSet")
	{
		auto DoubleIndexHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, DoubleIndexSet);
		Group.AddPropertyRow(GET_CHILD_PROPERTY(DoubleIndexHandle, FVoxelPaintMaterialDoubleIndexSet, IndexA));
		Group.AddPropertyRow(GET_CHILD_PROPERTY(DoubleIndexHandle, FVoxelPaintMaterialDoubleIndexSet, IndexB));
		Group.AddPropertyRow(GET_CHILD_PROPERTY(DoubleIndexHandle, FVoxelPaintMaterialDoubleIndexSet, Blend));
		Group.AddPropertyRow(GET_CHILD_PROPERTY(DoubleIndexHandle, FVoxelPaintMaterialDoubleIndexSet, bSetIndexA));
		Group.AddPropertyRow(GET_CHILD_PROPERTY(DoubleIndexHandle, FVoxelPaintMaterialDoubleIndexSet, bSetIndexB));
		Group.AddPropertyRow(GET_CHILD_PROPERTY(DoubleIndexHandle, FVoxelPaintMaterialDoubleIndexSet, bSetBlend));
	}
	else if (Type == "DoubleIndexBlend")
	{
		Group.AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, Index));
		Group.AddPropertyRow(GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, Amount));
	}
	else
	{
		ensure(false);
	}
}

#undef GET_CHILD_PROPERTY
#undef LOCTEXT_NAMESPACE