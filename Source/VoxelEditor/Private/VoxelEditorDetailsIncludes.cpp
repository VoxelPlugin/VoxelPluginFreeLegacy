// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelEditorDetailsIncludes.h"

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