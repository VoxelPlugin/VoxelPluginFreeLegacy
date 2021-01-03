// Copyright 2021 Phyronnaz

#include "Details/VoxelIntervalCustomization.h"
#include "VoxelInterval.h"
#include "VoxelEditorDetailsIncludes.h"

void FVoxelIntervalCustomization::GetSortedChildren(TSharedRef<IPropertyHandle> PropertyHandle, TArray<TSharedRef<IPropertyHandle>>& OutChildren)
{
	OutChildren.Add(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFloatInterval, Min));
	OutChildren.Add(GET_CHILD_PROPERTY(PropertyHandle, FVoxelFloatInterval, Max));
}