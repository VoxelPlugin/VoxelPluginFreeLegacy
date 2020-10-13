// Copyright 2020 Phyronnaz

#include "Details/VoxelSpawnerConfigSpawnerCustomization.h"
#include "VoxelMinimal.h"
#include "VoxelSpawners/VoxelSpawnerConfig.h"

#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "IPropertyUtilities.h"
#include "PropertyCustomizationHelpers.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"

#define GET_CHILD_PROPERTY_IMPL(PropertyHandle, Class, Property) PropertyHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(Class, Property)).ToSharedRef()
#define GET_CHILD_PROPERTY(Class, Property) GET_CHILD_PROPERTY_IMPL(PropertyHandle, Class, Property)

void FVoxelSpawnerConfigSpawnerCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	const auto& SpawnerTypeEnum = *StaticEnum<EVoxelSpawnerType>();
	
	const auto TypeHandle = GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, SpawnerType);
	
	EVoxelSpawnerType Type;
	{
		FString TypeString;
		if (!ensure(TypeHandle->GetValueAsFormattedString(TypeString) == FPropertyAccess::Success)) return;

		const int64 TypeValue = SpawnerTypeEnum.GetValueByNameString(TypeString);
		if (!ensure(TypeValue != -1)) return;

		Type = EVoxelSpawnerType(TypeValue);
	}

	const FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateLambda([&CustomizationUtils]()
	{
		auto Utilities = CustomizationUtils.GetPropertyUtilities();
		if (Utilities.IsValid())
		{
			Utilities->ForceRefresh();
		}
	});
	TypeHandle->SetOnPropertyValueChanged(RefreshDelegate);

	IDetailGroup& Group = ChildBuilder.AddGroup(TEXT("Spawner Config"), PropertyHandle->GetPropertyDisplayName());
	Group.HeaderRow()
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.MinDesiredWidth(FDetailWidgetRow::DefaultValueMinWidth)
			[
				TypeHandle->CreatePropertyValueWidget()
			]
		]
	    + SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(2.0f, 1.0f)
		[
			PropertyCustomizationHelpers::MakeInsertDeleteDuplicateButton(
				FExecuteAction::CreateLambda([=]() { PropertyHandle->GetParentHandle()->AsArray()->Insert(PropertyHandle->GetIndexInArray()); }),
				FExecuteAction::CreateLambda([=]() { PropertyHandle->GetParentHandle()->AsArray()->DeleteItem(PropertyHandle->GetIndexInArray()); }), 
				FExecuteAction::CreateLambda([=]() { PropertyHandle->GetParentHandle()->AsArray()->DuplicateItem(PropertyHandle->GetIndexInArray()); }))
		]
	];

	Group.AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, Spawner));
	Group.AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, Density));
	
	if (Type == EVoxelSpawnerType::Height)
	{
		Group.AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, HeightGraphOutputName_HeightOnly));
	}

	Group.AddWidgetRow()
	.NameContent()
	[
		GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, ChunkSize_EditorOnly)->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.MinDesiredWidth(FDetailWidgetRow::DefaultValueMinWidth)
			[
				GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, ChunkSize_EditorOnly)->CreatePropertyValueWidget()
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(5.f, 0.f, 0.f, 0.f)
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text_Lambda([=]()
			{
				FText Value;
				GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, LOD)->GetValueAsDisplayText(Value);
				return FText::Format(VOXEL_LOCTEXT("LOD: {0}"), Value);
			})
		]
	];

	Group.AddWidgetRow()
	.NameContent()
	[
		GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, GenerationDistanceInVoxels_EditorOnly)->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.MinDesiredWidth(FDetailWidgetRow::DefaultValueMinWidth)
			[
				GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, GenerationDistanceInVoxels_EditorOnly)->CreatePropertyValueWidget()
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(5.f, 0.f, 0.f, 0.f)
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text_Lambda([=]()
			{
				FText Value;
				GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, GenerationDistanceInChunks)->GetValueAsDisplayText(Value);
				return FText::Format(VOXEL_LOCTEXT("{0} chunks"), Value);
			})
		]
	];

	IDetailGroup& AdvancedGroup = Group.AddGroup("Advanced", VOXEL_LOCTEXT("Advanced"));

	AdvancedGroup.AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, bSave));
	AdvancedGroup.AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, bDoNotDespawn));
	AdvancedGroup.AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, Seed));

	AdvancedGroup.AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, RandomGenerator));
	AdvancedGroup.AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, Guid));
	
	if (Type == EVoxelSpawnerType::Ray)
	{
		AdvancedGroup.AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, DensityMultiplier_RayOnly));
	}
	if (Type == EVoxelSpawnerType::Height)
	{
		AdvancedGroup.AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, bComputeDensityFirst_HeightOnly));
		AdvancedGroup.AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, bCheckIfFloating_HeightOnly));
		AdvancedGroup.AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerConfigSpawner, bCheckIfCovered_HeightOnly));
	}
}

#undef GET_CHILD_PROPERTY