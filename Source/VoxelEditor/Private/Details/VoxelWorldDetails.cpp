// Copyright 2019 Phyronnaz

#include "VoxelWorldDetails.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelData.h"

#include "VoxelWorldEditor.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"

#include "VoxelEditorDetailsUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "DesktopPlatformModule.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Widgets/Input/SButton.h"

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
	World->UpdateCollisionProfile();

	World->PostEditChange();
	
	if (World->GetWorld())
	{
		if (World->GetWorld()->WorldType == EWorldType::Editor)
		{
			ADD_BUTTON_TO_CATEGORY(DetailLayout,
				"Voxel",
				LOCTEXT("Toggle", "Toggle"),
				LOCTEXT("ToggleWorldPreview", "Toggle World Preview"),
				LOCTEXT("Toggle", "Toggle"),
				this,
				&FVoxelWorldDetails::OnWorldPreviewToggle);
		}

		{
			SAssignNew(SaveButton, SButton)
			.ContentPadding(2)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked(this, &FVoxelWorldDetails::OnSave)
			.IsEnabled_Lambda([=]() { return World.IsValid() && World->IsCreated(); })
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("Save", "Save"))
			];

			SAssignNew(LoadButton, SButton)
			.ContentPadding(2)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked(this, &FVoxelWorldDetails::OnLoad)
			.IsEnabled_Lambda([=]() { return World.IsValid() && World->IsCreated(); })
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
		}

		{
			SAssignNew(SaveFileButton, SButton)
			.ContentPadding(2)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked(this, &FVoxelWorldDetails::OnSaveToFile)
			.IsEnabled_Lambda([=]() { return World.IsValid() && World->IsCreated(); })
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("SaveFile", "Save file"))
			];

			SAssignNew(LoadFileButton, SButton)
			.ContentPadding(2)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked(this, &FVoxelWorldDetails::OnLoadFromFile)
			.IsEnabled_Lambda([=]() { return World.IsValid() && World->IsCreated(); })
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("LoadFile", "Load file"))
			];

			DetailLayout.EditCategory("Voxel")
			.AddCustomRow(LOCTEXT("SaveLoadFile", "Save Load File"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("SaveLoadWorldFile", "Save/Load World from file"))
			]
			.ValueContent()
			.MaxDesiredWidth(125.f)
			.MinDesiredWidth(125.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SaveFileButton.ToSharedRef()
				]
				+ SHorizontalBox::Slot()
				[
					LoadFileButton.ToSharedRef()
				]
			];
		}
	}
}

FReply FVoxelWorldDetails::OnWorldPreviewToggle()
{
	if (World.IsValid())
	{
		if (World->IsCreated())
		{
			World->DestroyInEditor();
		}
		else
		{
			World->CreateInEditor(AVoxelWorldEditor::StaticClass());
		}

		GEditor->SelectActor(World.Get(), true, true, true, true);
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
			bDoIt = FMessageDialog::Open(EAppMsgType::YesNoCancel, LOCTEXT("Unsaved", "There are unsaved changes. Loading from Save Object will override them. Confirm?")) == EAppReturnType::Yes;
		}
		else if (World->GetData()->GetMaxHistory() != LastSaveHistoryPosition)
		{
			bDoIt = FMessageDialog::Open(EAppMsgType::YesNoCancel, LOCTEXT("UnsavedRedo", "There are unsaved changes in the redo history. Loading from Save Object will override them. Confirm?")) == EAppReturnType::Yes;
		}

		if (bDoIt)
		{
			UVoxelWorldSaveObject* SaveObject = World->GetSaveObject();
			if (SaveObject->Save.GetDepth() == -1)
			{
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InvalidSave", "Invalid save"));
			}else
			{
				FVoxelUncompressedWorldSave Save;
				UVoxelSaveUtilities::DecompressVoxelSave(SaveObject->Save, Save);
				World->LoadFromSave(Save);
				LastSaveHistoryPosition = 0;
			}
		}
	}
	return FReply::Handled();
}

FReply FVoxelWorldDetails::OnSave()
{
	UVoxelWorldSaveObject* SaveObject = World->GetSaveObject();
	if (World->IsCreated() && SaveObject)
	{		
		SaveObject->Modify();

		FVoxelUncompressedWorldSave Save;
		World->GetSave(Save);
		UVoxelSaveUtilities::CompressVoxelSave(Save, SaveObject->Save);

		LastSaveHistoryPosition = World->GetData()->GetHistoryPosition();
	}
	return FReply::Handled();
}

FReply FVoxelWorldDetails::OnSaveToFile()
{
	const FString DefaultPath = FPaths::GetProjectFilePath();

	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(SaveFileButton.ToSharedRef());
	check(ParentWindow.IsValid() && ParentWindow->GetNativeWindow().IsValid());
	void* ParentWindowHandle = ParentWindow->GetNativeWindow()->GetOSWindowHandle();

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	
	TArray<FString> OutFiles;
	if (DesktopPlatform->SaveFileDialog(ParentWindowHandle, TEXT("File to create"), FPaths::ProjectSavedDir(), World->GetName() + TEXT("_") + FDateTime::Now().ToString() + TEXT(".sav"), TEXT(".sav"), EFileDialogFlags::None, OutFiles))
	{
		check(OutFiles.Num() == 1);

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		FString AbsoluteFilePath = OutFiles[0];
		
		FBufferArchive Archive;
		{
			FVoxelCompressedWorldSave CompressedSave;
			FVoxelUncompressedWorldSave Save;
			World->GetSave(Save);
			UVoxelSaveUtilities::CompressVoxelSave(Save, CompressedSave);

			CompressedSave.Serialize(Archive);
		}

		if (FFileHelper::SaveArrayToFile(Archive, *AbsoluteFilePath))
		{
			FString Text = AbsoluteFilePath + TEXT(" was successfully created");
			FNotificationInfo Info(FText::FromString(Text));
			Info.ExpireDuration = 10.0f;
			FSlateNotificationManager::Get().AddNotification(Info);
		}
		else
		{
			FString Text = AbsoluteFilePath + TEXT(" was NOT successfully created");
			FNotificationInfo Info(FText::FromString(Text));
			Info.ExpireDuration = 10.0f;
			Info.CheckBoxState = ECheckBoxState::Unchecked;
			FSlateNotificationManager::Get().AddNotification(Info);
		}
	}
	
	return FReply::Handled();
}

FReply FVoxelWorldDetails::OnLoadFromFile()
{	
	const FString DefaultPath = FPaths::GetProjectFilePath();

	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(SaveFileButton.ToSharedRef());
	check(ParentWindow.IsValid() && ParentWindow->GetNativeWindow().IsValid());
	void* ParentWindowHandle = ParentWindow->GetNativeWindow()->GetOSWindowHandle();

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	
	TArray<FString> OutFiles;
	if (DesktopPlatform->OpenFileDialog(ParentWindowHandle, TEXT("File to open"), FPaths::ProjectSavedDir(), World->GetName() + TEXT(".sav"), TEXT(".sav"), EFileDialogFlags::None, OutFiles))
	{
		check(OutFiles.Num() == 1);

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		FString AbsoluteFilePath = OutFiles[0];		

		TArray<uint8> Array;
		if (!FFileHelper::LoadFileToArray(Array, *AbsoluteFilePath))
		{
			FString Text = AbsoluteFilePath + TEXT(" was NOT successfully loaded");
			FNotificationInfo Info(FText::FromString(Text));
			Info.ExpireDuration = 10.0f;
			Info.CheckBoxState = ECheckBoxState::Unchecked;
			FSlateNotificationManager::Get().AddNotification(Info);
			return FReply::Handled();
		}

		FMemoryReader Reader(Array);
		{
			FVoxelCompressedWorldSave CompressedSave;
			CompressedSave.Serialize(Reader);
			FVoxelUncompressedWorldSave Save;
			UVoxelSaveUtilities::DecompressVoxelSave(CompressedSave, Save);
			World->LoadFromSave(Save);
		}
		
		FString Text = AbsoluteFilePath + TEXT(" was successfully created");
		FNotificationInfo Info(FText::FromString(Text));
		Info.ExpireDuration = 10.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
	
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
