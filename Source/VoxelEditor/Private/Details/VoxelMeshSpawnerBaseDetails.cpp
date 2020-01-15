// Copyright 2020 Phyronnaz

#include "VoxelMeshSpawnerBaseDetails.h"
#include "VoxelSpawners/VoxelMeshSpawner.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

TSharedRef<IDetailCustomization> FVoxelMeshSpawnerBaseDetails::MakeInstance()
{
	return MakeShareable(new FVoxelMeshSpawnerBaseDetails());
}

class FFoliageTypeCustomizationHelpers
{
public:
	static void Edit(IDetailLayoutBuilder& DetailLayout)
	{
		static const FName PhysicsName("Physics");
		IDetailCategoryBuilder& PhysicsCategory = DetailLayout.EditCategory(PhysicsName);

		DetailLayout.EditCategory("Mesh", FText(), ECategoryPriority::Important);
		DetailLayout.EditCategory("Placement", FText(), ECategoryPriority::Important);
		DetailLayout.EditCategory("Placement - Offset", FText(), ECategoryPriority::Important);
		DetailLayout.EditCategory("Placement - Scale", FText(), ECategoryPriority::Important);
		DetailLayout.EditCategory("Placement - Rotation", FText(), ECategoryPriority::Important);
		DetailLayout.EditCategory("Instance Settings", FText(), ECategoryPriority::TypeSpecific);
		DetailLayout.EditCategory(PhysicsName, FText(), ECategoryPriority::Uncommon);

		const auto InstancedMeshSettingsHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelMeshSpawnerBase, InstancedMeshSettings));
		const auto BodyInstanceHandle = InstancedMeshSettingsHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelInstancedMeshSettings, BodyInstance));

		PhysicsCategory.AddProperty(BodyInstanceHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FBodyInstance, WalkableSlopeOverride)));
		PhysicsCategory.AddProperty(BodyInstanceHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FBodyInstance, PhysMaterialOverride)));
	}
};

void FVoxelMeshSpawnerBaseDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	FFoliageTypeCustomizationHelpers::Edit(DetailLayout);
}