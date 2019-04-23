// Copyright 2019 Phyronnaz

#include "VoxelWorldDetails.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelData.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelDataTools.h"

#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"

#include "VoxelEditorDetailsUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "DesktopPlatformModule.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Input/SButton.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "DetailLayoutBuilder.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "Voxel"

TSharedRef<IDetailCustomization> FVoxelWorldDetails::MakeInstance()
{
	return MakeShareable(new FVoxelWorldDetails());
}

void FVoxelWorldDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	FVoxelEditorDetailsUtils::EnableRealtime();
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailLayout.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() != 1)
	{
		return;
	}
	World = CastChecked<AVoxelWorld>(Objects[0].Get());;
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

	switch (World->UVConfig)
	{
	case EVoxelUVConfig::GlobalUVs:
		break;
	case EVoxelUVConfig::UseRGAsUVs:
	case EVoxelUVConfig::PackWorldUpInUVs:
	case EVoxelUVConfig::PerVoxelUVs:
	case EVoxelUVConfig::CustomFVoxelMaterial:
		DetailLayout.HideProperty(GET_MEMBER_NAME_CHECKED(AVoxelWorld, UVScale));
		break;
	default:
		check(false);
		break;
	}
	
	FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateLambda([&DetailLayout]()
	{			
		DetailLayout.ForceRefreshDetails();
	});
	DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(AVoxelWorld, MaterialConfig))->SetOnPropertyValueChanged(RefreshDelegate);
	DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(AVoxelWorld, UVConfig))->SetOnPropertyValueChanged(RefreshDelegate);

	if (World->GetWorld()) // Could be BP details
	{
		if (World->GetWorld()->WorldType == EWorldType::Editor)
		{
			FVoxelEditorDetailsUtils::AddButtonToCategory(
				DetailLayout,
				"Voxel - Preview",
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
							World->CreateForPreview();
						}

						GEditor->SelectActor(World.Get(), true, true, true, true);
					}

					return FReply::Handled();
				}));
			FVoxelEditorDetailsUtils::AddButtonToCategory(
				DetailLayout,
				"Voxel - Preview",
				LOCTEXT("Clear", "Clear"),
				LOCTEXT("ClearWorldData", "Clear World Data"),
				LOCTEXT("Clear", "Clear"),
				FOnClicked::CreateLambda([World = World]()
				{
					if (World.IsValid())
					{
						if (World->IsCreated())
						{
							if (FMessageDialog::Open(EAppMsgType::YesNoCancel, LOCTEXT("ClearDataWarning", "This will clear all the voxel world edits! Do you want to continue?"))
								== EAppReturnType::Yes)
							{
								{
									auto& Data = World->GetData();
									FVoxelReadWriteScopeLock Lock(Data, FIntBox::Infinite, "ClearData");
									Data.ClearData();
								}
								World->GetLODManager().UpdateBounds(FIntBox::Infinite);
							}
						}
					}

					return FReply::Handled();
				}));
		}

		FVoxelEditorDetailsUtils::AddButtonToCategory(
			DetailLayout,
			"Voxel - Save",
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
			"Voxel - Save",
			GET_MEMBER_NAME_CHECKED(AVoxelWorld, SaveObject));

		auto IsWorldCreatedDelegate = TAttribute<bool>::Create([=]() { return World.IsValid() && World->IsCreated(); });

		FVoxelEditorDetailsUtils::AddButtonToCategory(
			DetailLayout,
			"Voxel - Save",
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
			"Voxel - Save",
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
