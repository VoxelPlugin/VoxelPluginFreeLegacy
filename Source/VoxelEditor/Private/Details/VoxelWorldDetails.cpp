// Copyright 2018 Phyronnaz

#include "VoxelWorldDetails.h"
#include "VoxelWorld.h"
#include "VoxelData.h"

#include "VoxelWorldEditor.h"
#include "PropertyEditorModule.h"
#include "ModuleManager.h"

#include "VoxelEditorUtils.h"

#define LOCTEXT_NAMESPACE "VoxelWorldDetails"

TSharedRef<IDetailCustomization> FVoxelWorldDetails::MakeInstance()
{
	return MakeShareable(new FVoxelWorldDetails());
}

FVoxelWorldDetails::FVoxelWorldDetails()
	: LastSaveHistoryPosition(0)
{

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


		SaveButton =
		SNew(SButton)
		.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FVoxelWorldDetails::OnSave)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("Save", "Save"))
		];

		LoadButton =
		SNew(SButton)
		.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FVoxelWorldDetails::OnLoad)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("Load", "Load"))
		];

		DetailLayout.EditCategory("Voxel")
		.AddCustomRow(LOCTEXT("SaveLoad", "Save Load"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("SaveLoadWorld", "Save/Load World from Save Object"))
		]
		.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SaveButton.ToSharedRef()
			]
			+ SHorizontalBox::Slot()
			[
				LoadButton.ToSharedRef()
			]
		];
		
		SaveButton->SetEnabled(World->IsCreated());
		LoadButton->SetEnabled(World->IsCreated());
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

		SaveButton->SetEnabled(World->IsCreated());
		LoadButton->SetEnabled(World->IsCreated());

		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}

	return FReply::Handled();
}

FReply FVoxelWorldDetails::OnLoad()
{
	if (World->IsCreated() && World->GetSaveObject())
	{
		bool bDoIt = true;

		if (World->GetData()->GetHistoryPosition() != LastSaveHistoryPosition)
		{
			bDoIt = FMessageDialog::Open(EAppMsgType::YesNoCancel, LOCTEXT("", "There are unsaved changes. Loading from Save Object will override them. Confirm?")) == EAppReturnType::Yes;
		}
		else if (World->GetData()->GetMaxHistory() != LastSaveHistoryPosition)
		{
			bDoIt = FMessageDialog::Open(EAppMsgType::YesNoCancel, LOCTEXT("", "There are unsaved changes in the redo history. Loading from Save Object will override them. Confirm?")) == EAppReturnType::Yes;
		}

		if (bDoIt)
		{
			UVoxelWorldSaveObject* SaveObject = World->GetSaveObject();
			if (SaveObject->Save.LOD == World->GetLOD())
			{
				World->LoadFromSave(SaveObject->Save);
				LastSaveHistoryPosition = 0;
			}
			else
			{
				FFormatNamedArguments Arguments;
				Arguments.Add(TEXT("Depth"), FText::FromString(FString::FromInt(SaveObject->Save.LOD)));
				FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("", "Can't load the save: World Size is different (Save Octree Depth = {Depth})"), Arguments));
			}
		}
	}
	return FReply::Handled();
}

FReply FVoxelWorldDetails::OnSave()
{
	if (World->IsCreated() && World->GetSaveObject())
	{
		UVoxelWorldSaveObject* SaveObject = World->GetSaveObject();

		World->GetSave(SaveObject->Save);

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		// Notify the asset registry
		FAssetRegistryModule::AssetCreated(SaveObject);

		// Set the dirty flag so this package will get saved later
		SaveObject->MarkPackageDirty();

		LastSaveHistoryPosition = World->GetData()->GetHistoryPosition();
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE