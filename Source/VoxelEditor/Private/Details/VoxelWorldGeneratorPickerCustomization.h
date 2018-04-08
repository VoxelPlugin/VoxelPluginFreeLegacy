// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

struct FAssetData;
class IPropertyHandle;

class FVoxelWorldGeneratorPickerCustomization : public IPropertyTypeCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<class IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FVoxelWorldGeneratorPickerCustomization());
	}

	// IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	/** Respond to a selection change event from the combo box */
	void UpdateProperty();

private:
	/** Array of predefined resolutions */
	struct FPredefinedResolution
	{
		FText DisplayName;
		uint32 ResX, ResY;
	};
	TArray<TSharedPtr<FPredefinedResolution>> Resolutions;

	/** The index of the resolution we're currently displaying */
	int32 CurrentIndex;
	/** The text of the current selection */
	TSharedPtr<class STextBlock> CurrentText;
	/** The custom sliders to be hidden and shown based on combo box selection */
	TSharedPtr<class SWidget> CustomSliders;

	/** Property handles of the properties we're editing */
	TSharedPtr<class IPropertyHandle> PropertyHandle;
	TSharedPtr<class IPropertyHandle> ClassHandle, ObjectHandle, ClassOrObjectHandle;

	struct FClassOrObject
	{
		FText DisplayName;
	};

	TArray<TSharedPtr<FClassOrObject>> ClassOrObjectArray;
	TSharedPtr<class SWidget> ClassSlider;
	TSharedPtr<class SWidget> ObjectSlider;
};
