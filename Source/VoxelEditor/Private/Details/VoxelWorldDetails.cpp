// Copyright 2018 Phyronnaz

#include "VoxelWorldDetails.h"
#include "VoxelWorld.h"

#include "VoxelWorldEditor.h"
#include "PropertyEditorModule.h"
#include "ModuleManager.h"

#include "VoxelEditorUtils.h"

#define LOCTEXT_NAMESPACE "VoxelWorldDetails"

TSharedRef<IDetailCustomization> FVoxelWorldDetails::MakeInstance()
{
	return MakeShareable(new FVoxelWorldDetails());
}

void FVoxelWorldDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	World = FVoxelEditorDetailsUtils::GetCurrentObjectFromDetails<AVoxelWorld>(DetailLayout);

	World->PostEditChange();

	if (World->GetWorld())
	{
		ADD_BUTTON_TO_CATEGORY(DetailLayout,
			"Voxel",
			LOCTEXT("Toggle", "Toggle"),
			LOCTEXT("ToggleWorldPreview", "Toggle World Preview"),
			LOCTEXT("Toggle", "Toggle"),
			this,
			&FVoxelWorldDetails::OnWorldPreviewToggle)
	}
}

FReply FVoxelWorldDetails::OnWorldPreviewToggle()
{
	if (World.IsValid())
	{
		World->VoxelWorldEditorClass = AVoxelWorldEditor::StaticClass();

		if (World->IsCreated())
		{
			World->DestroyInEditor();
		}
		else
		{
			World->CreateInEditor();
		}

		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE