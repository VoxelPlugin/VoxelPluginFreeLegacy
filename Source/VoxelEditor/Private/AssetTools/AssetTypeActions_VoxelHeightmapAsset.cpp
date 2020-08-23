// Copyright 2020 Phyronnaz

#include "AssetTools/AssetTypeActions_VoxelHeightmapAsset.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/UIAction.h"
#include "EditorStyleSet.h"
#include "EditorReimportHandler.h"

void FAssetTypeActions_VoxelHeightmapAsset::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	const auto Heightmaps = GetTypedWeakObjectPtrs<UVoxelHeightmapAsset>(InObjects);

	MenuBuilder.AddMenuEntry(
		VOXEL_LOCTEXT("Reimport"),
		VOXEL_LOCTEXT("Reimport the selected heightmaps(s)."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.AssetActions.ReimportAsset"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_VoxelHeightmapAsset::ExecuteReimport, Heightmaps),
			FCanExecuteAction::CreateSP(this, &FAssetTypeActions_VoxelHeightmapAsset::CanExecuteReimport, Heightmaps)
		)
	);
}

bool FAssetTypeActions_VoxelHeightmapAsset::CanExecuteReimport(const TArray<TWeakObjectPtr<UVoxelHeightmapAsset>> Objects) const
{
	for (auto& Object : Objects)
	{
		if (Object.IsValid() && Object->IsA<UVoxelHeightmapAssetUINT16>())
		{
			return true;
		}
	}
	return false;
}

void FAssetTypeActions_VoxelHeightmapAsset::ExecuteReimport(const TArray<TWeakObjectPtr<UVoxelHeightmapAsset>> Objects) const
{
	for (auto& Object : Objects)
	{
		if (Object.IsValid() && Object->IsA<UVoxelHeightmapAssetUINT16>())
		{
			FReimportManager::Instance()->Reimport(Object.Get(), /*bAskForNewFileIfMissing=*/true);
		}
	}
}