// Copyright 2020 Phyronnaz

#include "Details/VoxelGeneratorOutputPickerCustomization.h"
#include "VoxelFoliage/VoxelFoliage.h"
#include "VoxelEditorDetailsIncludes.h"
#include "VoxelEditorDetailsUtilities.h"

void FVoxelGeneratorOutputPickerCustomization::CustomizeHeader(
	const TSharedRef<IPropertyHandle> PropertyHandle, 
	FDetailWidgetRow& HeaderRow, 
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	const bool bIsDensityOutput = PropertyHandle->HasMetaData(STATIC_FNAME("DensityOutput"));
	NameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelGeneratorOutputPicker, Name)).ToSharedRef();
	
	const auto SetupDefault = [&]()
	{
		HeaderRow.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			NameHandle->CreatePropertyValueWidget()
		];
	};
	
	if (PropertyHandle->GetNumPerObjectValues() > 1)
	{
		SetupDefault();
		return;
	}

	TSharedPtr<IPropertyHandle> GeneratorHandle;
	{
		TSharedPtr<IPropertyHandle> ParentHandle = PropertyHandle;
		while (ParentHandle.IsValid())
		{
			const auto UseMainGeneratorHandle = ParentHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelFoliageDensity, bUseMainGenerator));
			if (bIsDensityOutput && UseMainGeneratorHandle)
			{
				UseMainGeneratorHandle->SetOnPropertyValueChanged(FVoxelEditorUtilities::MakeRefreshDelegate(CustomizationUtils));

				bool bUseMainGenerator = false;
				if (ensure(UseMainGeneratorHandle->GetValue(bUseMainGenerator) == FPropertyAccess::Success) && !bUseMainGenerator)
				{
					GeneratorHandle = ParentHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelFoliageDensity, CustomGenerator));
					if (ensure(GeneratorHandle))
					{
						break;
					}
				}
			}
			const auto MainGeneratorHandle = ParentHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(UVoxelFoliage, OutputPickerGenerator));
			if (MainGeneratorHandle)
			{
				GeneratorHandle = MainGeneratorHandle;
				break;
			}
			
			ParentHandle = ParentHandle->GetParentHandle();
		}
	}

	if (!GeneratorHandle.IsValid())
	{
		SetupDefault();
		return;
	}

	GeneratorHandle->SetOnPropertyValueChanged(FVoxelEditorUtilities::MakeRefreshDelegate(CustomizationUtils));

	void* Address = nullptr;
	if (!ensure(GeneratorHandle->GetValueData(Address) == FPropertyAccess::Success))
	{
		return;
	}

	FVoxelGeneratorPicker& Picker = *static_cast<FVoxelGeneratorPicker*>(Address);
	if (!Picker.IsValid())
	{
		SetupDefault();
		return;
	}

	FString OutputType = PropertyHandle->GetMetaData("OutputType");
	if (OutputType.IsEmpty())
	{
		OutputType = "float";
	}
	ensure(OutputType == "int" || OutputType == "float");
	
	const auto Outputs = Picker.GetGenerator()->GetGeneratorOutputs();
	const TArray<FName> ValidOutputNames = OutputType == "int" ? Outputs.IntOutputs : Outputs.FloatOutputs;

	FName Value;
	if (!ensure(NameHandle->GetValue(Value) == FPropertyAccess::Success))
	{
		return;
	}

	OptionsSource.Reset();
	for (auto& Name : ValidOutputNames)
	{
		OptionsSource.Add(MakeShared<FName>(Name));
	}
		
	const auto ValuePtrPtr = OptionsSource.FindByPredicate([&](auto& Ptr) { return *Ptr == Value; });
	ensure(ValuePtrPtr || !ValidOutputNames.Contains(Value));

	ComboBoxText = FVoxelEditorUtilities::CreateText(FText::FromName(Value), FSlateColor(ValuePtrPtr ? FColor::Black : FColor::Red));
	
	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SBox)
		.MinDesiredWidth(FDetailWidgetRow::DefaultValueMinWidth)
		[
			SNew(SComboBox<TSharedPtr<FName>>)
			.OptionsSource(&OptionsSource)
			.OnSelectionChanged(this, &FVoxelGeneratorOutputPickerCustomization::HandleComboBoxSelectionChanged)
			.OnGenerateWidget_Lambda([&](TSharedPtr<FName> InValue)
			{
				return FVoxelEditorUtilities::CreateText(FText::FromName(*InValue));
			})
			.InitiallySelectedItem(ValuePtrPtr ? *ValuePtrPtr : TSharedPtr<FName>())
			[
				ComboBoxText.ToSharedRef()
			]
		]
	];
}

void FVoxelGeneratorOutputPickerCustomization::HandleComboBoxSelectionChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo) const
{
	if (ensure(NewSelection.IsValid()) && ensure(NameHandle.IsValid()) && ensure(ComboBoxText.IsValid()))
	{
		NameHandle->SetValue(*NewSelection);
		ComboBoxText->SetText(FText::FromName(*NewSelection));
		ComboBoxText->SetColorAndOpacity(FSlateColor(FColor::Black)); // If it's selected, it's always valid
	}
}
