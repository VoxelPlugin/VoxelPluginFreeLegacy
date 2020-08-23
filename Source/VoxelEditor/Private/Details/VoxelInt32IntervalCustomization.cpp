// Copyright 2020 Phyronnaz

#include "Details/VoxelInt32IntervalCustomization.h"
#include "VoxelMinimal.h"

void FVoxelInt32IntervalCustomization::GetSortedChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, TArray<TSharedRef<IPropertyHandle>>& OutChildren)
{
	TSharedPtr<IPropertyHandle> IntervalChildren[2];

	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);

	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		const TSharedRef<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();
		const FName PropertyName = ChildHandle->GetProperty()->GetFName();

		if (PropertyName == STATIC_FNAME("Min"))
		{
			IntervalChildren[0] = ChildHandle;
		}
		else
		{
			check(PropertyName == STATIC_FNAME("Max"));
			IntervalChildren[1] = ChildHandle;
		}
	}

	OutChildren.Add(IntervalChildren[0].ToSharedRef());
	OutChildren.Add(IntervalChildren[1].ToSharedRef());
}