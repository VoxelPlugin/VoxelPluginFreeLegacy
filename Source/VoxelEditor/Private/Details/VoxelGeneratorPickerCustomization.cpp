// Copyright 2020 Phyronnaz

#include "VoxelGeneratorPickerCustomization.h"

#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGenerators/VoxelGeneratorParameters.h"
#include "VoxelEditorDetailsIncludes.h"

#include "EdGraphSchema_K2.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"

void FVoxelGeneratorPickerCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	const auto ClassHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelGeneratorPicker, Class));
	const auto ObjectHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelGeneratorPicker, Object));
	const auto TypeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelGeneratorPicker, Type));
	
	ClassHandle->SetOnPropertyValueChanged(FVoxelEditorUtilities::MakeRefreshDelegate(CustomizationUtils));
	ObjectHandle->SetOnPropertyValueChanged(FVoxelEditorUtilities::MakeRefreshDelegate(CustomizationUtils));
	TypeHandle->SetOnPropertyValueChanged(FVoxelEditorUtilities::MakeRefreshDelegate(CustomizationUtils));

	ComboBoxArray.Add(MakeSharedCopy(EVoxelGeneratorPickerType::Class));
	ComboBoxArray.Add(MakeSharedCopy(EVoxelGeneratorPickerType::Object));

	PickerType = GetPicker(*PropertyHandle).Type;

	{
		void* Address = nullptr;
		if (!ensure(TypeHandle->GetValueData(Address) == FPropertyAccess::Success) || !ensure(Address))
		{
			return;
		}

		PickerType = *static_cast<EVoxelGeneratorPickerType*>(Address);
	}

	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.HAlign(HAlign_Fill)
	.MaxDesiredWidth(TOptional<float>())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			[
				SNew(SComboBox<TSharedPtr<EVoxelGeneratorPickerType>>)
				.IsEnabled(!TypeHandle->IsEditConst())
				.OptionsSource(&ComboBoxArray)
				.OnSelectionChanged_Lambda([=](TSharedPtr<EVoxelGeneratorPickerType> Value, ESelectInfo::Type)
				{
					PickerType = *Value;
					TypeHandle->SetValueFromFormattedString(UEnum::GetDisplayValueAsText(PickerType).ToString());
				})
				.OnGenerateWidget_Lambda([=](TSharedPtr<EVoxelGeneratorPickerType> Value)
				{
					return
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(UEnum::GetDisplayValueAsText(*Value));
				})
				.InitiallySelectedItem(PickerType == EVoxelGeneratorPickerType::Class ? ComboBoxArray[0] : ComboBoxArray[1])
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text_Lambda([=]()
					{
						return UEnum::GetDisplayValueAsText(PickerType);
					})
				]
			]
		]

		+ SHorizontalBox::Slot()
		[
			SNew(SBox)
			.HAlign(HAlign_Left)
			.Visibility_Lambda([=]()
			{
				return PickerType == EVoxelGeneratorPickerType::Class ? EVisibility::Visible : EVisibility::Collapsed;
			})
			[
				ClassHandle->CreatePropertyValueWidget()
			]
		]

		+ SHorizontalBox::Slot()
		[
			SNew(SBox)
			.HAlign(HAlign_Left)
			.Visibility_Lambda([=]()
			{
				return PickerType == EVoxelGeneratorPickerType::Object ? EVisibility::Visible : EVisibility::Collapsed;
			})
			[
				ObjectHandle->CreatePropertyValueWidget()
			]
		]
	];
}

class FVoxelGeneratorPickerCustomizationChildBuilder : public IDetailCustomNodeBuilder
{
public:
	const FString Name;
	const TWeakObjectPtr<UObject> Object;
	TArray<FName> PropertyNames;
	
	FVoxelGeneratorPickerCustomizationChildBuilder(const FString& Name, const TWeakObjectPtr<UObject>& Object)
		: Name(Name)
		, Object(Object)
	{
	}
	
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override {}
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override
	{
		NodeRow
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(Name))
			.Font(FEditorStyle::GetFontStyle(PropertyEditorConstants::CategoryFontStyle))
		];
	}
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override
	{
		if (!ensure(Object.IsValid())) return;
		
		for (auto& PropertyName : PropertyNames)
		{
			if (ensure(Object->GetClass()->FindPropertyByName(PropertyName)))
			{
				ChildrenBuilder.AddExternalObjectProperty({ Object.Get() }, PropertyName, FAddPropertyParams());
			}
		}
	}

	virtual void Tick(float DeltaTime) override {}
	virtual bool RequiresTick() const override { return false; }
	virtual bool InitiallyCollapsed() const override { return false; }
	virtual FName GetName() const override { return *Name; }
};

void FVoxelGeneratorPickerCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Important: Picker might actually be a FVoxelTransformableGeneratorPicker
	// We CANNOT call GetInstance on it
	FVoxelGeneratorPicker& Picker = GetPicker(*PropertyHandle);

	if (!Picker.IsValid() || PropertyHandle->GetNumOuterObjects() != 1)
	{
		return;
	}

	TArray<FVoxelGeneratorParameter> Parameters;
	if (auto* Generator = Picker.GetGenerator())
	{
		Parameters = Generator->GetParameters();
	}

	TMap<FName, FVoxelGeneratorParameter> NameToParameter;
	for (auto& Parameter : Parameters)
	{
		NameToParameter.Add(Parameter.Id, Parameter);
	}

	// Needs to match FVoxelCppConfig::BuildExposedVariablesArray
	Parameters.Sort([](const FVoxelGeneratorParameter& A, const FVoxelGeneratorParameter& B)
	{
		if (A.Priority != B.Priority)
		{
			return A.Priority < B.Priority;
		}
		return A.Id.LexicalLess(B.Id);
	});

	Parameters.RemoveAll([](const FVoxelGeneratorParameter& Parameter) { return Parameter.MetaData.Contains("HideInGenerator"); });

	TMap<FName, TArray<FVoxelGeneratorParameter>> CategoriesToParameters;
	for (auto& Parameter : Parameters)
	{
		CategoriesToParameters.FindOrAdd(*Parameter.Category).Add(Parameter);	
	}

	CategoriesToParameters.KeySort([](const FName& A, const FName& B)
	{
		if (A.IsNone() != B.IsNone())
		{
			// Put empty on top
			return A.IsNone() > B.IsNone();
		}
		return A.LexicalLess(B);
	});
	
	UVoxelGeneratorPickerEditorData* EditorData = Cast<UVoxelGeneratorPickerEditorData>(Picker.EditorData);
	if (!EditorData ||
		!ensure(EditorData->Blueprint) ||
		!ensure(EditorData->BlueprintInstance) ||
		EditorData->GeneratorObject != Picker.GetObject() ||
		EditorData->Parameters != Parameters)
	{
		auto& BlueprintPool = GetMutableDefault<UVoxelGeneratorPickerBlueprintPool>()->Blueprints;

		if (EditorData)
		{
			if (EditorData->Blueprint)
			{
				// Pool blueprints, as we can't have a BP deleted before its class
				BlueprintPool.Add(EditorData->Blueprint);
			}
			if (EditorData->BlueprintInstance) 
			{
				EditorData->BlueprintInstance->MarkPendingKill();
			}
			
			EditorData->GeneratorObject = nullptr;
			EditorData->Parameters = {};
			EditorData->Blueprint = nullptr;
			EditorData->BlueprintInstance = nullptr;
		}
		else
		{
			EditorData = NewObject<UVoxelGeneratorPickerEditorData>();
			Picker.EditorData = EditorData;
		}

		UBlueprint* Blueprint = nullptr;
		if (BlueprintPool.Num() > 0)
		{
			Blueprint = BlueprintPool.Pop();
		}
		if (!Blueprint)
		{
			Blueprint = NewObject<UBlueprint>(GetTransientPackage(), NAME_None, RF_Transient);
		}
		
		Blueprint->NewVariables.Reset();
		FBlueprintEditorUtils::RemoveGeneratedClasses(Blueprint);
		
		{
			Blueprint->ParentClass = UObject::StaticClass();

			for (auto& Parameter : Parameters)
			{
				FBlueprintEditorUtils::AddMemberVariable(Blueprint, Parameter.Id, GetParameterPinType(Parameter.Type));
			}

			for (auto& Variable : Blueprint->NewVariables)
			{
				auto* Parameter = NameToParameter.Find(Variable.VarName);
				if (!ensure(Parameter)) continue;

				if (!Parameter->Category.IsEmpty())
				{
					Variable.Category = FText::FromString(Parameter->Category);
				}

				Variable.MetaDataArray.Add(FBPVariableMetaDataEntry(TEXT("Tooltip"), Parameter->ToolTip + "\n\nUnique Name: " + Parameter->Id.ToString()));

				for (auto& It : Parameter->MetaData)
				{
					Variable.MetaDataArray.Add(FBPVariableMetaDataEntry(It.Key, It.Value));
				}
			}

			FKismetEditorUtilities::CompileBlueprint(Blueprint,
				EBlueprintCompileOptions::SkipGarbageCollection |
				EBlueprintCompileOptions::BatchCompile |
				EBlueprintCompileOptions::SkipFiBSearchMetaUpdate);

			// Fixup the defaults on the CDO
			// They're ignored when set on the variables above
			for (TFieldIterator<FProperty> It(Blueprint->GeneratedClass); It; ++It)
			{
				auto* Property = *It;

				auto* Parameter = NameToParameter.Find(Property->GetFName());
				if (!ensure(Parameter)) continue;

				auto* CDO = Blueprint->GeneratedClass->GetDefaultObject();
				Property->ImportText(*Parameter->DefaultValue, It->ContainerPtrToValuePtr<void>(CDO), PPF_None, CDO);
			}
		}

		EditorData->GeneratorObject = Picker.GetObject();
		EditorData->Parameters = Parameters;
		EditorData->Blueprint = Blueprint;
		EditorData->BlueprintInstance = NewObject<UObject>(GetTransientPackage(), Blueprint->GeneratedClass, NAME_None, RF_Transient | RF_Transactional);
	}

	auto* BlueprintInstance = EditorData->BlueprintInstance;
	
	// Apply the overrides
	for (TFieldIterator<FProperty> It(BlueprintInstance->GetClass()); It; ++It)
	{
		auto* Property = *It;

		auto* Value = Picker.Parameters.Find(Property->GetFName());
		if (!Value) continue;

		Property->ImportText(**Value, It->ContainerPtrToValuePtr<void>(BlueprintInstance), PPF_None, BlueprintInstance);
	}
	
	FCoreUObjectDelegates::OnObjectPropertyChanged.Add(MakeWeakPtrDelegate(PropertyHandle, 
	[=, &Picker, Handle = &PropertyHandle.Get()](UObject* InObject, FPropertyChangedEvent& PropertyChangedEvent)
	{
		if (BlueprintInstance != InObject) return;

		if (GIsTransacting)
		{
			// OnObjectPropertyChanged is called when undoing
			return;
		}

		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
		{
			return;
		}

		if (!ensure(Handle) || !ensure(IsValid(BlueprintInstance)))
		{
			return;
		}
		
		FScopedTransaction Transaction(TEXT("VoxelGeneratorParameters"), VOXEL_LOCTEXT("Edit Generator Parameters"), BlueprintInstance);

		Handle->NotifyPreChange();
		
		for (TFieldIterator<FProperty> It(BlueprintInstance->GetClass()); It; ++It)
		{
			auto* Property = *It;

			auto* Parameter = NameToParameter.Find(Property->GetFName());
			if (!ensure(Parameter)) continue;

			FString Value;
			Property->ExportTextItem(Value, It->ContainerPtrToValuePtr<void>(BlueprintInstance), nullptr, BlueprintInstance, PPF_None);

			if (Parameter->DefaultValue == Value)
			{
				Picker.Parameters.Remove(Parameter->Id);
			}
			else
			{
				Picker.Parameters.Add(Parameter->Id, Value);
			}
		}

		Handle->NotifyPostChange();
	}));
	
	{
		const auto ButtonHBox = SNew(SHorizontalBox);
		
		ChildBuilder.AddCustomRow({})
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(TOptional<float>()) // This is needed to not be clipped by the engine
		[
			ButtonHBox
		];

		ButtonHBox->AddSlot()
		.Padding(0.f, 0.f, 4.f, 0.f)
		[
			SNew(SButton)
			.ContentPadding(2)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.ToolTipText(VOXEL_LOCTEXT("Refresh the parameters list"))
			.OnClicked_Lambda([&Picker, Utilities = MakeWeakPtr(CustomizationUtils.GetPropertyUtilities())]()
			{
				// Force blueprint recompile
				if (auto* EditorData = Cast<UVoxelGeneratorPickerEditorData>(Picker.EditorData))
				{
					EditorData->GeneratorObject = nullptr;
				}
				if (auto Pinned = Utilities.Pin()) Pinned->ForceRefresh();
				return FReply::Handled();
			})
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(VOXEL_LOCTEXT("Refresh"))
			]
		];

		ButtonHBox->AddSlot()
		.Padding(0.f, 0.f, 4.f, 0.f)
		[
			SNew(SButton)
			.ContentPadding(2)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.ToolTipText(VOXEL_LOCTEXT("Reset all parameters to their default values"))
			.OnClicked_Lambda([&Picker, PropertyHandle, Utilities = MakeWeakPtr(CustomizationUtils.GetPropertyUtilities())]()
			{
				{
					FScopedTransaction Transaction(TEXT("VoxelGeneratorParameters"), VOXEL_LOCTEXT("Reset"), nullptr);
					PropertyHandle->NotifyPreChange();
					Picker.Parameters.Reset();
					// Force blueprint recompile
					if (auto* EditorData = Cast<UVoxelGeneratorPickerEditorData>(Picker.EditorData))
					{
						EditorData->GeneratorObject = nullptr;
					}
					PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
				}
				
				if (auto Pinned = Utilities.Pin()) Pinned->ForceRefresh();
				return FReply::Handled();
			})
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(VOXEL_LOCTEXT("Clear"))
			]
		];
		
		if (Picker.IsObject())
		{
			ButtonHBox->AddSlot()
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SButton)
				.ContentPadding(2)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.ToolTipText(VOXEL_LOCTEXT("Store the current parameters in the object as the new defaults. If it's a voxel graph, will change the parameter nodes values"))
				.OnClicked_Lambda([&Picker, PropertyHandle, Utilities = MakeWeakPtr(CustomizationUtils.GetPropertyUtilities())]()
				{
					auto* Generator = Picker.GetGenerator();
					check(Generator);

					{
						FScopedTransaction Transaction(TEXT("VoxelGeneratorParameters"), VOXEL_LOCTEXT("Set Defaults"), Generator);
						PropertyHandle->NotifyPreChange();

						Generator->Modify();
						Generator->ApplyParameters(Picker.Parameters);

						Picker.Parameters.Reset();

						PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
					}

					if (auto Pinned = Utilities.Pin()) Pinned->ForceRefresh();

					return FReply::Handled();
				})
				.IsEnabled_Lambda([&Picker]()
				{
					return Picker.Parameters.Num() > 0 && Picker.GetGenerator();
				})
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(VOXEL_LOCTEXT("Set Defaults"))
				]
			];
		}
	}

	// ChildBuilder.AddExternalObjects is broken, so add properties manually
	for (auto& CategoryIt : CategoriesToParameters)
	{
		if (CategoryIt.Key.IsNone())
		{
			for (auto& Parameter : CategoryIt.Value)
			{
				ChildBuilder.AddExternalObjectProperty({ BlueprintInstance }, Parameter.Id, FAddPropertyParams());
			}
		}
		else
		{
			auto Builder = MakeShared<FVoxelGeneratorPickerCustomizationChildBuilder>(CategoryIt.Value[0].Category, BlueprintInstance);
			for (auto& Parameter : CategoryIt.Value)
			{
				Builder->PropertyNames.Add(Parameter.Id);
			}
			ChildBuilder.AddCustomBuilder(Builder);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelGeneratorPicker& FVoxelGeneratorPickerCustomization::GetPicker(IPropertyHandle& Handle)
{
	void* Address = nullptr;
	if (!ensure(Handle.GetValueData(Address) == FPropertyAccess::Success) || !ensure(Address))
	{
		static FVoxelGeneratorPicker Static;
		return Static;
	}
	
	return *static_cast<FVoxelGeneratorPicker*>(Address);
}

FEdGraphPinType FVoxelGeneratorPickerCustomization::GetParameterPinType(const FVoxelGeneratorParameterType& ParameterType)
{
	FEdGraphPinType BaseType = FEdGraphPinType::GetPinTypeForTerminalType(GetParameterTerminalPinType(ParameterType));

	switch (ParameterType.ContainerType)
	{
	default: ensure(false);
	case EVoxelGeneratorParameterContainerType::None:
	{
		BaseType.ContainerType = EPinContainerType::None;
		return BaseType;
	}
	case EVoxelGeneratorParameterContainerType::Array:
	{
		BaseType.ContainerType = EPinContainerType::Array;
		return BaseType;
	}
	case EVoxelGeneratorParameterContainerType::Set:
	{
		BaseType.ContainerType = EPinContainerType::Set;
		return BaseType;
	}
	case EVoxelGeneratorParameterContainerType::Map:
	{
		BaseType.ContainerType = EPinContainerType::Map;
		BaseType.PinValueType = GetParameterTerminalPinType(ParameterType.ValueType);
		return BaseType;
	}
	}
}

FEdGraphTerminalType FVoxelGeneratorPickerCustomization::GetParameterTerminalPinType(const FVoxelGeneratorParameterTerminalType& ParameterType)
{
	const auto Make = [](FName TerminalCategory, FName TerminalSubCategory, TWeakObjectPtr<UObject> TerminalSubCategoryObject)
	{
		FEdGraphTerminalType Result;
		Result.TerminalCategory = TerminalCategory;
		Result.TerminalSubCategory = TerminalSubCategory;
		Result.TerminalSubCategoryObject = TerminalSubCategoryObject;
		return Result;
	};
	
	switch (ParameterType.PropertyType)
	{
	default: ensure(false); return FEdGraphTerminalType();
	case EVoxelGeneratorParameterPropertyType::Float:
	{
		return Make(UEdGraphSchema_K2::PC_Float, NAME_None, nullptr);
	}
	case EVoxelGeneratorParameterPropertyType::Int:
	{
		return Make(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr);
	}
	case EVoxelGeneratorParameterPropertyType::Bool:
	{
		return Make(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr);
	}
	case EVoxelGeneratorParameterPropertyType::Name:
	{
		return Make(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr);
	}
	case EVoxelGeneratorParameterPropertyType::Object:
	{
		auto* Class = FindObject<UClass>(ANY_PACKAGE, *ParameterType.PropertyClass.ToString());
		ensure(Class);
		return Make(UEdGraphSchema_K2::PC_Object, ParameterType.PropertyClass, Class);
	}
	case EVoxelGeneratorParameterPropertyType::Struct:
	{
		auto* Struct = FindObject<UScriptStruct>(ANY_PACKAGE, *ParameterType.PropertyClass.ToString());
		ensure(Struct);
		return Make(UEdGraphSchema_K2::PC_Struct, ParameterType.PropertyClass, Struct);
	}
	}
}
