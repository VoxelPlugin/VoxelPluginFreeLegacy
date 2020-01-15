// Copyright 2020 Phyronnaz

#include "VoxelAssetActorDetails.h"
#include "VoxelPlaceableItems/VoxelAssetActor.h"
#include "VoxelData/VoxelData.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelWorld.h"
#include "VoxelEditorDetailsUtilities.h"
#include "VoxelScopedTransaction.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "Voxel"

TSharedRef<IDetailCustomization> FVoxelAssetActorDetails::MakeInstance()
{
	return MakeShareable(new FVoxelAssetActorDetails());
}

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
		LOCTEXT("AssetUpdate", "Voxel Asset Update"),
		LOCTEXT("UpdateRender", "Update Render"),
		LOCTEXT("Update", "Update"),
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
				&& AssetActor->WorldGenerator.IsValid()
				&& AssetActor->PreviewWorld;
		}));

	FVoxelEditorUtilities::AddButtonToCategory(
		DetailLayout,
		"Editor Tools",
		LOCTEXT("Stamp", "Stamp"),
		LOCTEXT("Stamp", "Stamp"),
		LOCTEXT("Stamp", "Stamp"),
		false,
		FOnClicked::CreateLambda([=]()
		{
			FVoxelScopedTransaction Transaction(AssetActor->PreviewWorld, STATIC_FNAME("Stamp"));
			const auto Bounds = AssetActor->AddItemToData(
				AssetActor->PreviewWorld, 
				&AssetActor->PreviewWorld->GetData());
			AssetActor->PreviewWorld->GetLODManager().UpdateBounds(Bounds);
			AssetActor->PreviewWorld->GetData().SaveFrame(Bounds);
			return FReply::Handled();
		}),
		TAttribute<bool>::Create([=]()
		{
			return AssetActor.IsValid()
				&& AssetActor->GetWorld()->WorldType == EWorldType::Editor
				&& AssetActor->WorldGenerator.IsValid()
				&& AssetActor->PreviewWorld
				&& AssetActor->PreviewWorld->IsCreated();
		}));
}
#undef LOCTEXT_NAMESPACE