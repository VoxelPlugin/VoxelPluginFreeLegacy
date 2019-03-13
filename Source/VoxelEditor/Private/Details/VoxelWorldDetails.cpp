// Copyright 2019 Phyronnaz

#include "VoxelWorldDetails.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelDataTools.h"

#include "VoxelWorldEditor.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"

#include "VoxelEditorDetailsUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "DesktopPlatformModule.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Widgets/Input/SButton.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

#define LOCTEXT_NAMESPACE "Voxel"

TSharedRef<IDetailCustomization> FVoxelWorldDetails::MakeInstance()
{
	return MakeShareable(new FVoxelWorldDetails());
}

void FVoxelWorldDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	World = FVoxelEditorDetailsUtils::GetCurrentObjectFromDetails<AVoxelWorld>(DetailLayout);
	World->UpdateCollisionProfile();
	World->PostEditChange();

	switch (World->MaterialConfig)
	{
	case EVoxelMaterialConfig::RGB:
		DetailLayout.HideProperty(GET_MEMBER_NAME_CHECKED(AVoxelWorld, MaterialCollection));
		break;
	case EVoxelMaterialConfig::SingleIndex:
	case EVoxelMaterialConfig::DoubleIndex:
		DetailLayout.HideProperty(GET_MEMBER_NAME_CHECKED(AVoxelWorld, VoxelMaterial));
		DetailLayout.HideProperty(GET_MEMBER_NAME_CHECKED(AVoxelWorld, TessellatedVoxelMaterial));
		break;
	default:
		check(false);
		break;
	}
	
	TSharedRef<IPropertyHandle> PropertiesHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(AVoxelWorld, MaterialConfig));
	FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateLambda([&DetailLayout]()
	{			
		DetailLayout.ForceRefreshDetails();
	});
	PropertiesHandle->SetOnPropertyValueChanged(RefreshDelegate);

	if (World->GetWorld()) // Could be BP details
	{
		if (World->GetWorld()->WorldType == EWorldType::Editor)
		{
			FVoxelEditorDetailsUtils::AddButtonToCategory(
				DetailLayout,
				"Voxel",
				LOCTEXT("Toggle", "Toggle"),
				LOCTEXT("ToggleWorldPreview", "Toggle World Preview"),
				LOCTEXT("Toggle", "Toggle"),
				FOnClicked::CreateLambda([World = World]()
				{
					if (World.IsValid())
					{
						if (World->IsCreated())
						{
							World->DestroyWorld();
						}
						else
						{
							World->CreateForPreview(AVoxelWorldEditor::StaticClass());
						}

						GEditor->SelectActor(World.Get(), true, true, true, true);
					}

					return FReply::Handled();
				}));
		}

		FVoxelEditorDetailsUtils::AddButtonToCategory(
			DetailLayout,
			"Voxel",
			LOCTEXT("Load", "Load"),
			LOCTEXT("LoadFromSave", "Load from Save Object"),
			LOCTEXT("Load", "Load"),
			FOnClicked::CreateLambda([World = World]()
			{
				if (World.IsValid() && ensure(World->IsCreated()) && ensure(World->SaveObject))
				{
					World->LoadFromSaveObjectEditor();
				}
				return FReply::Handled();
			}),
			TAttribute<bool>::Create([=]()
			{
				return World.IsValid() && World->IsCreated() && World->SaveObject;
			}));
		
		DetailLayout.HideProperty(GET_MEMBER_NAME_CHECKED(AVoxelWorld, SaveObject));
		FVoxelEditorDetailsUtils::AddPropertyToCategory(
			DetailLayout,
			"Voxel",
			GET_MEMBER_NAME_CHECKED(AVoxelWorld, SaveObject));

		auto IsWorldCreatedDelegate = TAttribute<bool>::Create([=]() { return World.IsValid() && World->IsCreated(); });

		FVoxelEditorDetailsUtils::AddButtonToCategory(
			DetailLayout,
			"Voxel",
			LOCTEXT("SaveFile", "Save File"),
			LOCTEXT("SaveToFile", "Save to File"),
			LOCTEXT("Save", "Save"),
			FOnClicked::CreateLambda([World = World]()
			{
				if (World.IsValid() && ensure(World->IsCreated()))
				{
					FString FilePath = World->GetDefaultFilePath();
					if (FilePath.IsEmpty())
					{
						TArray<FString> OutFiles;
						if (FDesktopPlatformModule::Get()->SaveFileDialog(
							FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
							TEXT("File to open"),
							FPaths::ProjectSavedDir(),
							"",
							TEXT("Voxel Save (*.voxelsave)|*.voxelsave"),
							EFileDialogFlags::None,
							OutFiles))
						{
							FilePath = OutFiles[0];
						}
					}
					FText Error;
					if (!World->SaveToFile(FilePath, Error))
					{
						FMessageDialog::Open(EAppMsgType::Ok, Error);
					}
				}
				return FReply::Handled();
			}),
			IsWorldCreatedDelegate);

		FVoxelEditorDetailsUtils::AddButtonToCategory(
			DetailLayout,
			"Voxel",
			LOCTEXT("LoadFile", "Load File"),
			LOCTEXT("LoadFromFile", "Load from File"),
			LOCTEXT("Load", "Load"),
			FOnClicked::CreateLambda([World = World]()
			{
				if (World.IsValid() && ensure(World->IsCreated()))
				{
					TArray<FString> OutFiles;
					if (FDesktopPlatformModule::Get()->OpenFileDialog(
						FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
						TEXT("File to open"),
						FPaths::ProjectSavedDir(),
						"",
						TEXT("Voxel Save (*.voxelsave)|*.voxelsave"),
						EFileDialogFlags::None,
						OutFiles))
					{
						check(OutFiles.Num() == 1);
						FText Error;
						if (!World->LoadFromFile(OutFiles[0], Error))
						{
							FMessageDialog::Open(EAppMsgType::Ok, Error);
						}
					}
				}
				return FReply::Handled();
			}),
			IsWorldCreatedDelegate);
	}
}

#undef LOCTEXT_NAMESPACE
