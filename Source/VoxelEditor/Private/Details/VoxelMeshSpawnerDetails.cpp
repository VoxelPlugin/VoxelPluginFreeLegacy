// Copyright 2020 Phyronnaz

#include "VoxelMeshSpawnerDetails.h"
#include "VoxelSpawners/VoxelMeshSpawner.h"

#include "DetailLayoutBuilder.h"

void FVoxelMeshSpawnerDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	DetailLayout.EditCategory("General Settings", FText(), ECategoryPriority::Important);
	DetailLayout.EditCategory("Actor Settings", FText(), ECategoryPriority::Important);
	DetailLayout.EditCategory("Instance Settings", FText(), ECategoryPriority::Important);
	DetailLayout.EditCategory("Placement", FText(), ECategoryPriority::Important);
	DetailLayout.EditCategory("Placement - Offset", FText(), ECategoryPriority::Important);
	DetailLayout.EditCategory("Placement - Scale", FText(), ECategoryPriority::Important);
	DetailLayout.EditCategory("Placement - Rotation", FText(), ECategoryPriority::Important);
	
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailLayout.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() == 1)
	{
		auto* MeshSpawner = CastChecked<UVoxelMeshSpawner>(Objects[0]);
		switch (MeshSpawner->InstanceRandom)
		{
		case EVoxelMeshSpawnerInstanceRandom::Random:
		case EVoxelMeshSpawnerInstanceRandom::VoxelMaterial:
			DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(UVoxelMeshSpawner, ColorOutputName));
			break;
		case EVoxelMeshSpawnerInstanceRandom::ColorOutput:
			break;
		default:
			ensure(false);
			break;
		}

		const FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateLambda([&DetailLayout]()
		{
			DetailLayout.ForceRefreshDetails();
		});
		DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelMeshSpawner, InstanceRandom))->SetOnPropertyValueChanged(RefreshDelegate);
	}
}