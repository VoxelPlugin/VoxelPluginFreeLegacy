// Copyright 2020 Phyronnaz

#include "VoxelFoliageScaleCustomization.h"
#include "VoxelFoliage/VoxelFoliage.h"
#include "VoxelEditorDetailsIncludes.h"

void FVoxelFoliageScaleCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

void FVoxelFoliageScaleCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	auto TypeHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageScale, Scaling);
	auto ScaleXHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageScale, ScaleX);
	auto ScaleYHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageScale, ScaleY);
	auto ScaleZHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelFoliageScale, ScaleZ);

	FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateLambda([&CustomizationUtils]()
	{
		auto Utilities = CustomizationUtils.GetPropertyUtilities();
		if (Utilities.IsValid())
		{
			Utilities->ForceRefresh();
		}
	});
	TypeHandle->SetOnPropertyValueChanged(RefreshDelegate);
	
	ChildBuilder.AddProperty(TypeHandle);
	
	FString Type;
	TypeHandle->GetValueAsFormattedString(Type);
	if (Type == "Uniform")
	{
		ChildBuilder.AddProperty(ScaleXHandle).DisplayName(VOXEL_LOCTEXT("Scale"));
	}
	else if (Type == "Free")
	{
		ChildBuilder.AddProperty(ScaleXHandle);
		ChildBuilder.AddProperty(ScaleYHandle);
		ChildBuilder.AddProperty(ScaleZHandle);
	}
	else if (Type == "LockXY")
	{
		ChildBuilder.AddProperty(ScaleXHandle).DisplayName(VOXEL_LOCTEXT("Scale XY"));
		ChildBuilder.AddProperty(ScaleZHandle).DisplayName(VOXEL_LOCTEXT("Scale Z"));
	}
	else
	{
		ensure(false);
	}
}