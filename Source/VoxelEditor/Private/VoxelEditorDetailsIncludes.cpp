// Copyright 2020 Phyronnaz

#include "VoxelEditorDetailsIncludes.h"
#include "UserInterface/PropertyEditor/PropertyEditorConstants.cpp"

FSimpleDelegate FVoxelEditorUtilities::MakeRefreshDelegate(const IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	return FSimpleDelegate::CreateLambda([Utilities = MakeWeakPtr(CustomizationUtils.GetPropertyUtilities())]()
	{
		auto Pinned = Utilities.Pin();
		if (Pinned.IsValid())
		{
			Pinned->ForceRefresh();
		}
	});
}