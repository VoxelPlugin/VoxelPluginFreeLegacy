// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

enum class EVoxelPaintMaterialType : uint8;
class STextBlock;

class FVoxelPaintMaterialCustomization : public IPropertyTypeCustomization
{
public:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	private:
	TArray<TSharedPtr<EVoxelPaintMaterialType>> OptionsSource;
	TSharedPtr<STextBlock> ComboBoxText;
	TSharedPtr<IPropertyHandle> TypeHandle;
	
	void HandleComboBoxSelectionChanged(TSharedPtr<EVoxelPaintMaterialType> NewSelection, ESelectInfo::Type SelectInfo) const;
};

class FVoxelPaintMaterial_MaterialCollectionChannelCustomization : public IPropertyTypeCustomization
{
public:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override {}
};