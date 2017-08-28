// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VoxelActions.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/SlateStyle.h"
#include "VoxelAsset.h"
#include "VoxelEditorToolkit.h"


#define LOCTEXT_NAMESPACE "AssetTypeActions"


/* FVoxelActions constructors
 *****************************************************************************/

FVoxelActions::FVoxelActions(const TSharedRef<ISlateStyle>& InStyle) : Style(InStyle)
{ }


/* FAssetTypeActions_Base overrides
 *****************************************************************************/

bool FVoxelActions::CanFilter()
{
	return true;
}


void FVoxelActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);

	/*auto Voxels = GetTypedWeakObjectPtrs<UVoxel>(InObjects);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("Voxel_ReverseText", "Reverse Text"),
		LOCTEXT("Voxel_ReverseTextToolTip", "Reverse the text stored in the selected text asset(s)."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([=]{
				for (auto& Voxel : Voxels)
				{
					if (Voxel.IsValid() && !Voxel->Text.IsEmpty())
					{
						Voxel->Text = FText::FromString(Voxel->Text.ToString().Reverse());
						Voxel->PostEditChange();
						Voxel->MarkPackageDirty();
					}
				}
			}),
			FCanExecuteAction::CreateLambda([=] {
				for (auto& Voxel : Voxels)
				{
					if (Voxel.IsValid() && !Voxel->Text.IsEmpty())
					{
						return true;
					}
				}
				return false;
			})
		)
	);*/
}


uint32 FVoxelActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}


FText FVoxelActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_Voxel", "Voxel Asset");
}


UClass* FVoxelActions::GetSupportedClass() const
{
	return UVoxelAsset::StaticClass();
}


FColor FVoxelActions::GetTypeColor() const
{
	return FColor::White;
}


bool FVoxelActions::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}


void FVoxelActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
		? EToolkitMode::WorldCentric
		: EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Voxel = Cast<UVoxelAsset>(*ObjIt);

		if (Voxel != nullptr)
		{
			TSharedRef<FVoxelEditorToolkit> EditorToolkit = MakeShareable(new FVoxelEditorToolkit(Style));
			EditorToolkit->Initialize(Voxel, Mode, EditWithinLevelEditor);
		}
	}
}


#undef LOCTEXT_NAMESPACE
