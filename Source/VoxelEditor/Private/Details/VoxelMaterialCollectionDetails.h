// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"
#include "Widgets/Input/SButton.h"
#include "PropertyHandle.h"

class UVoxelMaterialCollection;

class FVoxelMaterialCollectionDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	FVoxelMaterialCollectionDetails();

private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

	FReply OnGenerateSingleMaterials();
	FReply OnGenerateDoubleMaterials();
	FReply OnGenerateTripleMaterials();
	FReply OnApplyPhysicalMaterials();

private:
	TWeakObjectPtr<UVoxelMaterialCollection> Collection;
};

class FVoxelMaterialCollectionElementCustomization : public IPropertyTypeCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<class IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FVoxelMaterialCollectionElementCustomization());
	}

	// IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	TArray<UObject*> Outers;

	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<IPropertyHandle> IndexHandle;
	TSharedPtr<IPropertyHandle> MaterialHandle;
	TSharedPtr<IPropertyHandle> PhysicalMaterialHandle;
	TSharedPtr<IPropertyHandle> ChildrenHandle;
	
	bool ArePhysicalMaterialsHidden() const;
	bool HasChildren() const;
	
	EVisibility NoChildrenOnlyVisibility() const { return HasChildren() ? EVisibility::Hidden : EVisibility::Visible; }
	EVisibility NoChildrenOnlyPhysicalMaterialsVisibility() const { return ArePhysicalMaterialsHidden() ? EVisibility::Hidden : NoChildrenOnlyVisibility(); }
	EVisibility PhysicalMaterialsVisibility() const { return ArePhysicalMaterialsHidden() ? EVisibility::Hidden : EVisibility::Visible; }

	float NoChildrenOnlyPhysicalMaterialsMaxHeight() const { return (HasChildren() || ArePhysicalMaterialsHidden()) ? 1 : 1000; }
	float PhysicalMaterialsMaxHeight() const { return ArePhysicalMaterialsHidden() ? 1 : 1000; }

	void GenerateArrayElementWidget(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder);
};