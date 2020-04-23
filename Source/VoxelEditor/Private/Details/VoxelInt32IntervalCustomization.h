// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Customizations/MathStructCustomizations.h"

class FVoxelInt32IntervalCustomization : public FMathStructCustomization
{
public:
	static TSharedRef<class IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FVoxelInt32IntervalCustomization());
	}

	virtual void GetSortedChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, TArray<TSharedRef<IPropertyHandle>>& OutChildren) override;
};