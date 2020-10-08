// Copyright 2020 Phyronnaz

#include "Details/VoxelSpawnerOutputNameCustomization.h"
#include "VoxelMinimal.h"
#include "VoxelEditorDetailsUtilities.h"
#include "VoxelSpawners/VoxelSpawnerConfig.h"
#include "VoxelSpawners/VoxelSpawnerOutputsConfig.h"

#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IPropertyUtilities.h"
#include "Widgets/Input/SComboBox.h"

void FVoxelSpawnerOutputNameCustomization::CustomizeHeader(
	const TSharedRef<IPropertyHandle> PropertyHandle, 
	FDetailWidgetRow& HeaderRow, 
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	NameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelSpawnerOutputName, Name)).ToSharedRef();

	TSharedPtr<IPropertyHandle> OutputsHandle;
	{
		auto ParentHandle = PropertyHandle;
		while (!OutputsHandle.IsValid() && ensure(ParentHandle->GetParentHandle().IsValid()))
		{
			ParentHandle = ParentHandle->GetParentHandle().ToSharedRef();
			OutputsHandle = ParentHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(UVoxelSpawnerConfig, GeneratorOutputs));
		}
	}

	if (!ensure(OutputsHandle.IsValid()))
	{
		return;
	}

	const FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateLambda([&CustomizationUtils]()
	{
		auto Utilities = CustomizationUtils.GetPropertyUtilities();
		if (Utilities.IsValid())
		{
			Utilities->ForceRefresh();
		}
	});
	OutputsHandle->SetOnPropertyValueChanged(RefreshDelegate);

	UObject* OutputsObject = nullptr;
	if (!ensure(OutputsHandle->GetValue(OutputsObject) == FPropertyAccess::Success))
	{
		return;
	}

	UVoxelSpawnerOutputsConfig* Outputs = Cast<UVoxelSpawnerOutputsConfig>(OutputsObject);
	if (!Outputs)
	{
		HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MaxDesiredWidth(250.f)
		[
			FVoxelEditorUtilities::CreateText(VOXEL_LOCTEXT("Invalid Generator Outputs"), FSlateColor(FColor::Red))
		];
		return;
	}

	const TArray<FName> ValidOutputNames = Outputs->GetFloatOutputs();

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
			.OnSelectionChanged(this, &FVoxelSpawnerOutputNameCustomization::HandleComboBoxSelectionChanged)
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

void FVoxelSpawnerOutputNameCustomization::HandleComboBoxSelectionChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (ensure(NewSelection.IsValid()) && ensure(NameHandle.IsValid()) && ensure(ComboBoxText.IsValid()))
	{
		NameHandle->SetValue(*NewSelection);
		ComboBoxText->SetText(FText::FromName(*NewSelection));
		ComboBoxText->SetColorAndOpacity(FSlateColor(FColor::Black)); // If it's selected, it's always valid
	}
}