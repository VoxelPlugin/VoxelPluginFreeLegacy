// Copyright 2020 Phyronnaz

#include "VoxelLandscapeCollectionDetails.h"
#include "VoxelEditorDetailsUtilities.h"
#include "VoxelEditorDetailsIncludes.h"

#include "VoxelRender/MaterialCollections/VoxelLandscapeMaterialCollection.h"

void FVoxelLandscapeCollectionDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailLayout.GetObjectsBeingCustomized(Objects);
		
	FVoxelEditorUtilities::AddButtonToCategory(
		DetailLayout,
		"Config",
		VOXEL_LOCTEXT("Build All Permutations"),
		VOXEL_LOCTEXT("Build All Permutations"),
		VOXEL_LOCTEXT("Build"),
		VOXEL_LOCTEXT(
			"Build all the permutations that can occur at runtime. You need to do this if you're getting checkerboards in packaged/standalone games.\n"
			"This might create a lot of materials, decrease MaxMaterialsToBlendAtOnce or add layers to LayersToIgnore to reduce compile times"),
		false,
		MakeLambdaDelegate([=]()
		{
			for (auto& Object : Objects)
			{
				if (auto* Collection = Cast<UVoxelLandscapeMaterialCollection>(Object.Get()))
				{
					Collection->BuildAllPermutations();
				}
			}
			
			return FReply::Handled();
		}));
		
	FVoxelEditorUtilities::AddButtonToCategory(
		DetailLayout,
		"Config",
		VOXEL_LOCTEXT("Clear Cache"),
		VOXEL_LOCTEXT("Clear Cache"),
		VOXEL_LOCTEXT("Clear"),
		VOXEL_LOCTEXT("Clears the material cache"),
		true,
		MakeLambdaDelegate([=]()
		{
			for (auto& Object : Objects)
			{
				if (auto* Collection = Cast<UVoxelLandscapeMaterialCollection>(Object.Get()))
				{
					Collection->Modify();
					Collection->MaterialCache.Empty();
				}
			}
			
			return FReply::Handled();
		}));
}
