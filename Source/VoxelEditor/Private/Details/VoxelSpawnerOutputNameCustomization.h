// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

class STextBlock;

class FVoxelSpawnerOutputNameCustomization : public IPropertyTypeCustomization
{
public:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override {}

private:
	TArray<TSharedPtr<FName>> OptionsSource;
	TSharedPtr<STextBlock> ComboBoxText;
	TSharedPtr<IPropertyHandle> NameHandle;
	
	void HandleComboBoxSelectionChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo);
};