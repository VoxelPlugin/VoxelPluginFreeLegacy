// Copyright 2020 Phyronnaz

#include "VoxelBasicSpawnerScaleSettingsCustomization.h"
#include "VoxelSpawners/VoxelBasicSpawner.h"

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "IPropertyUtilities.h"

#define LOCTEXT_NAMESPACE "Voxel"

void FVoxelBasicSpawnerScaleSettingsCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

#define GET_CHILD_PROPERTY(PropertyHandle, Class, Property) PropertyHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(Class, Property)).ToSharedRef()

void FVoxelBasicSpawnerScaleSettingsCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	auto TypeHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelBasicSpawnerScaleSettings, Scaling);
	auto ScaleXHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelBasicSpawnerScaleSettings, ScaleX);
	auto ScaleYHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelBasicSpawnerScaleSettings, ScaleY);
	auto ScaleZHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelBasicSpawnerScaleSettings, ScaleZ);

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
		ChildBuilder.AddProperty(ScaleXHandle).DisplayName(LOCTEXT("Scale", "Scale"));
	}
	else if (Type == "Free")
	{
		ChildBuilder.AddProperty(ScaleXHandle);
		ChildBuilder.AddProperty(ScaleYHandle);
		ChildBuilder.AddProperty(ScaleZHandle);
	}
	else if (Type == "LockXY")
	{
		ChildBuilder.AddProperty(ScaleXHandle).DisplayName(LOCTEXT("Scale XY", "Scale XY"));
		ChildBuilder.AddProperty(ScaleZHandle).DisplayName(LOCTEXT("Scale Z", "Scale Z"));
	}
	else
	{
		ensure(false);
	}
}

#undef LOCTEXT_NAMESPACE