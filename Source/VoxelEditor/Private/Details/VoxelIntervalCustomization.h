// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Customizations/MathStructCustomizations.h"

class FVoxelIntervalCustomization : public FMathStructCustomization
{
public:
	virtual void GetSortedChildren(TSharedRef<IPropertyHandle> PropertyHandle, TArray<TSharedRef<IPropertyHandle>>& OutChildren) override;
};