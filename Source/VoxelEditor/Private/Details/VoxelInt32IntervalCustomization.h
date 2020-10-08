// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Customizations/MathStructCustomizations.h"

class FVoxelInt32IntervalCustomization : public FMathStructCustomization
{
public:
	virtual void GetSortedChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, TArray<TSharedRef<IPropertyHandle>>& OutChildren) override;
};