// Copyright 2020 Phyronnaz

#include "VoxelAssetActorDetails.h"
#include "VoxelPlaceableItems/Actors/VoxelAssetActor.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelWorld.h"
#include "VoxelEditorDetailsUtilities.h"
#include "VoxelScopedTransaction.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

void FVoxelAssetActorDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	FVoxelEditorUtilities::EnableRealtime();
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailLayout.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() != 1)
	{
		return;
	}
	AssetActor = CastChecked<AVoxelAssetActor>(Objects[0].Get());

	DetailLayout.EditCategory("Editor Tools", FText(), ECategoryPriority::Important);
	DetailLayout.EditCategory("Placeable Item Actor Settings", FText(), ECategoryPriority::Important);

	FVoxelEditorUtilities::AddButtonToCategory(
		DetailLayout,
		"Preview Settings",
		VOXEL_LOCTEXT("Voxel Asset Update"),
		VOXEL_LOCTEXT("Update Render"),
		VOXEL_LOCTEXT("Update"),
		false,
		FOnClicked::CreateLambda([=]()
		{
			if (AssetActor.IsValid())
			{
				AssetActor->UpdatePreview();
			}
			return FReply::Handled();
		}),
		TAttribute<bool>::Create([=]()
		{
			return AssetActor.IsValid()
				&& AssetActor->GetWorld()
				&& AssetActor->GetWorld()->WorldType == EWorldType::Editor
				&& AssetActor->Generator.IsValid()
				&& AssetActor->PreviewWorld;
		}));

	FVoxelEditorUtilities::AddButtonToCategory(
		DetailLayout,
		"Editor Tools",
		VOXEL_LOCTEXT("Stamp"),
		VOXEL_LOCTEXT("Stamp"),
		VOXEL_LOCTEXT("Stamp"),
		false,
		FOnClicked::CreateLambda([=]()
		{
			FVoxelScopedTransaction Transaction(AssetActor->PreviewWorld, STATIC_FNAME("Stamp"), EVoxelChangeType::Edit);
			const auto Bounds = AssetActor->AddItemToData(
				AssetActor->PreviewWorld,
				&AssetActor->PreviewWorld->GetData());
			UVoxelBlueprintLibrary::UpdateBounds(AssetActor->PreviewWorld, Bounds);
			UVoxelBlueprintLibrary::SaveFrame(AssetActor->PreviewWorld);
			return FReply::Handled();
		}),
		TAttribute<bool>::Create([=]()
		{
			return AssetActor.IsValid()
				&& AssetActor->Generator.IsValid()
				&& AssetActor->PreviewWorld
				&& AssetActor->PreviewWorld->IsCreated();
		}));
}