// Copyright 2020 Phyronnaz

#include "Details/VoxelSpawnerDensityCustomization.h"
#include "VoxelMinimal.h"
#include "VoxelSpawners/VoxelSpawnerConfig.h"

#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "IPropertyUtilities.h"

#define GET_CHILD_PROPERTY(Class, Property) PropertyHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(Class, Property)).ToSharedRef()

void FVoxelSpawnerDensityCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	const auto& DensityTypeEnum = *StaticEnum<EVoxelSpawnerDensityType>();
	
	const auto TypeHandle = GET_CHILD_PROPERTY(FVoxelSpawnerDensity, Type);
	
	EVoxelSpawnerDensityType Type;
	{
		FString TypeString;
		if (!ensure(TypeHandle->GetValueAsFormattedString(TypeString) == FPropertyAccess::Success)) return;

		const int64 TypeValue = DensityTypeEnum.GetValueByNameString(TypeString);
		if (!ensure(TypeValue != -1)) return;

		Type = EVoxelSpawnerDensityType(TypeValue);
	}

	// Make sure to do that after the possible SetValue
	const FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateLambda([&CustomizationUtils]()
	{
		auto Utilities = CustomizationUtils.GetPropertyUtilities();
		if (Utilities.IsValid())
		{
			Utilities->ForceRefresh();
		}
	});
	TypeHandle->SetOnPropertyValueChanged(RefreshDelegate);

	IDetailGroup* Group = &ChildBuilder.AddGroup(TEXT("Spawner Density Type"), PropertyHandle->GetPropertyDisplayName());
	Group->HeaderRow()
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		TypeHandle->CreatePropertyValueWidget()
	];

	switch (Type)
	{
	default: ensure(false);
	case EVoxelSpawnerDensityType::Constant:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerDensity, Constant));
		break;
	}
	case EVoxelSpawnerDensityType::GeneratorOutput:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerDensity, GeneratorOutputName));
		break;
	}
	case EVoxelSpawnerDensityType::MaterialRGBA:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerDensity, RGBAChannel));
		break;
	}
	case EVoxelSpawnerDensityType::MaterialUVs:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerDensity, UVChannel));
		Group->AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerDensity, UVAxis));
		break;
	}
	case EVoxelSpawnerDensityType::MaterialFiveWayBlend:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerDensity, FiveWayBlendChannel));
		break;
	}
	case EVoxelSpawnerDensityType::SingleIndex:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerDensity, SingleIndexChannels));
		break;
	}
	case EVoxelSpawnerDensityType::MultiIndex:
	{
		Group->AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerDensity, MultiIndexChannels));
		break;
	}
	}
	Group->AddPropertyRow(GET_CHILD_PROPERTY(FVoxelSpawnerDensity, Transform));
}

#undef GET_CHILD_PROPERTY