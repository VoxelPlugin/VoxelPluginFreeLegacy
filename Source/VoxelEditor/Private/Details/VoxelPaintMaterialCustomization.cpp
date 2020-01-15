// Copyright 2020 Phyronnaz

#include "Details/VoxelPaintMaterialCustomization.h"
#include "VoxelTools/VoxelPaintMaterial.h"
#include "VoxelEditorDetailsUtilities.h"

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "IDetailGroup.h"
#include "Widgets/Input/SComboBox.h"

#define LOCTEXT_NAMESPACE "Voxel"

void FVoxelPaintMaterialCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

#define GET_CHILD_PROPERTY(PropertyHandle, Class, Property) PropertyHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(Class, Property)).ToSharedRef()

void FVoxelPaintMaterialCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TypeHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, Type);
	const auto RestrictTypeHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, bRestrictType);
	const auto MaterialConfigToRestrictToHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, MaterialConfigToRestrictTo);

	const FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateLambda([&CustomizationUtils]()
	{
		auto Utilities = CustomizationUtils.GetPropertyUtilities();
		if (Utilities.IsValid())
		{
			Utilities->ForceRefresh();
		}
	});
	TypeHandle->SetOnPropertyValueChanged(RefreshDelegate);

	FString TypeString;
	FString MaterialConfigToRestrictToString;
	
	EVoxelPaintMaterialType Type;
	bool bRestrictType = false;
	EVoxelMaterialConfig MaterialConfigToRestrictTo;
	
	{
		if (!ensure(TypeHandle->GetValueAsFormattedString(TypeString) == FPropertyAccess::Success)) return;
		if (!ensure(RestrictTypeHandle->GetValue(bRestrictType) == FPropertyAccess::Success)) return;
		if (!ensure(MaterialConfigToRestrictToHandle->GetValueAsFormattedString(MaterialConfigToRestrictToString) == FPropertyAccess::Success)) return;

		const int64 TypeValue = GET_STATIC_UENUM(EVoxelPaintMaterialType).GetValueByNameString(TypeString);
		if (!ensure(TypeValue != -1)) return;
		Type = EVoxelPaintMaterialType(TypeValue);

		const int64 MaterialConfigValue = GET_STATIC_UENUM(EVoxelMaterialConfig).GetValueByNameString(MaterialConfigToRestrictToString);
		if (!ensure(MaterialConfigValue != -1)) return;
		MaterialConfigToRestrictTo = EVoxelMaterialConfig(MaterialConfigValue);
	}
	
	TSharedPtr<SWidget> TypeWidget;
	if (bRestrictType)
	{
		OptionsSource.Reset();
		OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::UVs));
		if (MaterialConfigToRestrictTo == EVoxelMaterialConfig::RGB)
		{
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::RGB));
		}
		else if (MaterialConfigToRestrictTo == EVoxelMaterialConfig::SingleIndex)
		{
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::SingleIndex));
		}
		else
		{
			ensure(MaterialConfigToRestrictTo == EVoxelMaterialConfig::DoubleIndex);
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::DoubleIndexSet));
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::DoubleIndexBlend));
		}

		const auto TypePtrPtr = OptionsSource.FindByPredicate([&](auto& Ptr) { return *Ptr == Type; });

		const auto TypeText = GET_STATIC_UENUM(EVoxelPaintMaterialType).GetDisplayNameTextByValue(int64(Type));
		ComboBoxText = FVoxelEditorUtilities::CreateText(TypeText, FSlateColor(TypePtrPtr ? FColor::Black : FColor::Red));
		
		TypeWidget = SNew(SComboBox<TSharedPtr<EVoxelPaintMaterialType>>)
		.IsEnabled_Lambda([TypeHandle = TWeakPtr<IPropertyHandle>(TypeHandle)](){ return TypeHandle.IsValid() && !TypeHandle.Pin()->IsEditConst(); })
		.OptionsSource(&OptionsSource)
		.OnSelectionChanged(this, &FVoxelPaintMaterialCustomization::HandleComboBoxSelectionChanged)
		.OnGenerateWidget_Lambda([&](TSharedPtr<EVoxelPaintMaterialType> InValue)
		{
			return FVoxelEditorUtilities::CreateText(GET_STATIC_UENUM(EVoxelPaintMaterialType).GetDisplayNameTextByValue(int64(*InValue)));
		})
		.InitiallySelectedItem(TypePtrPtr ? *TypePtrPtr : TSharedPtr<EVoxelPaintMaterialType>())
		[
			ComboBoxText.ToSharedRef()
		];
	}
	else
	{
		TypeWidget = TypeHandle->CreatePropertyValueWidget();
	}

	const bool bShowOnlyInnerProperties = PropertyHandle->HasMetaData(STATIC_FNAME("ShowOnlyInnerProperties"));

	IDetailGroup* Group = nullptr;
	if (bShowOnlyInnerProperties)
	{
		ChildBuilder.AddCustomRow(LOCTEXT("Type", "Type"))
		.NameContent()
		[
			TypeHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			TypeWidget.ToSharedRef()
		];
	}
	else
	{
		Group = &ChildBuilder.AddGroup(TEXT("Paint Material Type"), PropertyHandle->GetPropertyDisplayName());
		Group->HeaderRow()
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			TypeWidget.ToSharedRef()
		];
	}
	
	const auto AddProperty = [&](const auto& InPropertyHandle)
	{
		if (bShowOnlyInnerProperties)
		{
			ChildBuilder.AddProperty(InPropertyHandle);
		}
		else
		{
			Group->AddPropertyRow(InPropertyHandle);
		}
	};

	if (Type == EVoxelPaintMaterialType::RGB)
	{
		const auto ColorHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, Color);
		AddProperty(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, Color));
		AddProperty(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bPaintR));
		AddProperty(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bPaintG));
		AddProperty(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bPaintB));
		AddProperty(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bPaintA));
	}
	else if (Type == EVoxelPaintMaterialType::SingleIndex)
	{
		AddProperty(GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, Index));
	}
	else if (Type == EVoxelPaintMaterialType::DoubleIndexSet)
	{
		const auto DoubleIndexHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, DoubleIndexSet);
		AddProperty(GET_CHILD_PROPERTY(DoubleIndexHandle, FVoxelPaintMaterialDoubleIndexSet, IndexA));
		AddProperty(GET_CHILD_PROPERTY(DoubleIndexHandle, FVoxelPaintMaterialDoubleIndexSet, IndexB));
		AddProperty(GET_CHILD_PROPERTY(DoubleIndexHandle, FVoxelPaintMaterialDoubleIndexSet, Blend));
		AddProperty(GET_CHILD_PROPERTY(DoubleIndexHandle, FVoxelPaintMaterialDoubleIndexSet, bSetIndexA));
		AddProperty(GET_CHILD_PROPERTY(DoubleIndexHandle, FVoxelPaintMaterialDoubleIndexSet, bSetIndexB));
		AddProperty(GET_CHILD_PROPERTY(DoubleIndexHandle, FVoxelPaintMaterialDoubleIndexSet, bSetBlend));
	}
	else if (Type == EVoxelPaintMaterialType::DoubleIndexBlend)
	{
		AddProperty(GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, Index));
	}
	else if (Type == EVoxelPaintMaterialType::UVs)
	{
		const auto UVHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, UV);
		AddProperty(GET_CHILD_PROPERTY(UVHandle, FVoxelPaintMaterialUV, Channel));
		AddProperty(GET_CHILD_PROPERTY(UVHandle, FVoxelPaintMaterialUV, UV));
		AddProperty(GET_CHILD_PROPERTY(UVHandle, FVoxelPaintMaterialUV, bPaintU));
		AddProperty(GET_CHILD_PROPERTY(UVHandle, FVoxelPaintMaterialUV, bPaintV));
	}
	else
	{
		ensure(false);
	}
}

#undef GET_CHILD_PROPERTY

void FVoxelPaintMaterialCustomization::HandleComboBoxSelectionChanged(TSharedPtr<EVoxelPaintMaterialType> NewSelection, ESelectInfo::Type SelectInfo) const
{
	if (ensure(NewSelection.IsValid()) && ensure(TypeHandle.IsValid()) && ensure(ComboBoxText.IsValid()))
	{
		const auto& Enum = GET_STATIC_UENUM(EVoxelPaintMaterialType);
		const int64 Value = int64(*NewSelection);
		TypeHandle->SetValueFromFormattedString(Enum.GetNameStringByValue(Value));
		ComboBoxText->SetText(Enum.GetDisplayNameTextByValue(Value));
	}
}

#undef LOCTEXT_NAMESPACE