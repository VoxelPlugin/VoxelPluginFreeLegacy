// Copyright 2020 Phyronnaz

#include "VoxelWorldDetails.h"
#include "VoxelWorld.h"
#include "VoxelStaticWorld.h"
#include "VoxelData/VoxelData.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelRender/VoxelMaterialInterface.h"
#include "VoxelEditorDetailsUtilities.h"
#include "VoxelMessages.h"

#include "Modules/ModuleManager.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Framework/Application/SlateApplication.h"
#include "DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/ScopedSlowTask.h"
#include "Serialization/MemoryReader.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "AssetRegistryModule.h"
#include "Editor.h"
#include "RawMesh.h"

// The sort order is being silly, so force set it
#include "Editor/PropertyEditor/Private/DetailCategoryBuilderImpl.h"

#define LOCTEXT_NAMESPACE "Voxel"

TSharedRef<IDetailCustomization> FVoxelWorldDetails::MakeInstance()
{
	return MakeShareable(new FVoxelWorldDetails(false));
}
TSharedRef<IDetailCustomization> FVoxelWorldDetails::MakeDataAssetEditorInstance()
{
	return MakeShareable(new FVoxelWorldDetails(true));
}

void FVoxelWorldDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	FVoxelEditorUtilities::EnableRealtime();
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailLayout.GetObjectsBeingCustomized(Objects);
	// Disabled as it makes BP compilation crash when calling PostEditChange
	//for (auto& Object : Objects)
	//{
	//	World->UpdateCollisionProfile();
	//	World->PostEditChange();
	//}
		
	// Material config specific setup
	if (Objects.Num() == 1)
	{
		auto* World = CastChecked<AVoxelWorld>(Objects[0]);
		switch (World->MaterialConfig)
		{
		case EVoxelMaterialConfig::RGB:
			DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, MaterialCollection));
			DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, MaterialsHardness));
			break;
		case EVoxelMaterialConfig::SingleIndex:
		case EVoxelMaterialConfig::DoubleIndex:
			DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, VoxelMaterial));
			DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, TessellatedVoxelMaterial));
			DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bUseAlphaAsHardness));
			break;
		default:
			ensure(false);
			break;
		}

		switch (World->UVConfig)
		{
		case EVoxelUVConfig::GlobalUVs:
			break;
		case EVoxelUVConfig::PackWorldUpInUVs:
		case EVoxelUVConfig::PerVoxelUVs:
			DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, UVScale));
			break;
		default:
			ensure(false);
			break;
		}
		
		const FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateLambda([&DetailLayout]()
		{
			DetailLayout.ForceRefreshDetails();
		});
		DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, MaterialConfig))->SetOnPropertyValueChanged(RefreshDelegate);
		DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, UVConfig))->SetOnPropertyValueChanged(RefreshDelegate);
	}

	if (bIsDataAssetEditor)
	{
		DetailLayout.HideCategory("Voxel - Save");
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, WorldGenerator));
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bCreateWorldAutomatically));
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, Seeds));
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bUseCameraIfNoInvokersFound));
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bEnableUndoRedo));
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bEnableCustomWorldRebasing));
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bMergeAssetActors));
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bMergeDisableEditsBoxes));
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bCreateGlobalPool));
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, ProcMeshClass));
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bRenderWorld));
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bStaticWorld));
		DetailLayout.HideCategory("Voxel - Spawners");
		DetailLayout.HideCategory("Physics");
		DetailLayout.HideCategory("Voxel - Collisions");
		DetailLayout.HideCategory("Voxel - Navmesh");
		DetailLayout.HideCategory("Voxel - Multiplayer");
		DetailLayout.HideCategory("Replication");
		DetailLayout.HideCategory("Input");
		DetailLayout.HideCategory("Actor");
		DetailLayout.HideCategory("Cooking");
		DetailLayout.HideCategory("TransformCommon");
		DetailLayout.HideCategory("ComponentReplication");
		DetailLayout.HideCategory("Variable");
		DetailLayout.HideCategory("Tick");
		DetailLayout.HideCategory("Voxel - Preview");
		DetailLayout.HideCategory("Voxel - Bake");
	}

	{
		// Component settings not affecting the voxel world
		DetailLayout.HideCategory("Lighting");
		DetailLayout.HideCategory("Tags");
		DetailLayout.HideCategory("Activation");
		DetailLayout.HideCategory("Rendering");
		DetailLayout.HideCategory("AssetUserData");
		DetailLayout.HideCategory("Mobile");

		// No HLOD for voxels
		DetailLayout.HideCategory("HLOD");

		// Manually handling those
		DetailLayout.HideCategory("Collision");

		const auto SortCategory = [&](FName Name, uint32 Order, bool bCollapsed, FString NewName = {})
		{
			FText NewNameText;
			if (!NewName.IsEmpty())
			{
				NewNameText = FText::FromString(NewName);
			}
			auto& Builder = static_cast<FDetailCategoryImpl&>(DetailLayout.EditCategory(Name, NewNameText));
			Builder.SetSortOrder(Order);
			Builder.InitiallyCollapsed(bCollapsed);
		};

		uint32 Order = 1000;
		SortCategory("Voxel - Preview", Order++, false);
		SortCategory("Voxel - Save", Order++, true);
		SortCategory("Voxel - General", Order++, false);
		SortCategory("Voxel - World Size", Order++, false);
		SortCategory("Voxel - Rendering", Order++, false);
		SortCategory("Voxel - Materials", Order++, false);
		SortCategory("Voxel - Spawners", Order++, true);
		SortCategory("Physics", Order++, true, "Voxel - Physics");
		SortCategory("Voxel - Collisions", Order++, true);
		SortCategory("Voxel - Navmesh", Order++, true);
		SortCategory("Voxel - LOD Settings", Order++, true);
		SortCategory("Voxel - Performance", Order++, true);
		SortCategory("Voxel - Multiplayer", Order++, true);
		SortCategory("Voxel - Bake", Order++, true);
		SortCategory("Replication", Order++, true);
		SortCategory("Input", Order++, true);
		SortCategory("Actor", Order++, true);
		SortCategory("Cooking", Order++, true);
		SortCategory("VirtualTexture", Order++, true);
	}

	const auto CreateWorldsDelegate = [Objects](auto Lambda)
	{
		return FOnClicked::CreateLambda([=]()
			{
				for (auto& Object : Objects)
				{
					auto* World = Cast<AVoxelWorld>(Object);
					if (World)
					{
						Lambda(*World);
					}
				}
				return FReply::Handled();
			});
	};
	const auto CreateWorldsEnabledDelegate = [Objects](auto Lambda)
	{
		return TAttribute<bool>::Create([=]()
			{
				for (auto& Object : Objects)
				{
					auto* World = Cast<AVoxelWorld>(Object);
					if (World)
					{
						if (!Lambda(*World))
						{
							return false;
						}
					}
				}
				return true;
			});
	};

	FVoxelEditorUtilities::AddButtonToCategory(
		DetailLayout,
		"Voxel - General",
		LOCTEXT("FillSeed", "FillSeedFromGenerator"),
		LOCTEXT("FillSeeds", "Fill Seeds From Generator"),
		LOCTEXT("Fill", "Fill Seeds"),
		false,
		CreateWorldsDelegate([](AVoxelWorld& World)
		{
			auto* WorldGenerator = World.WorldGenerator.GetWorldGenerator();
			if (WorldGenerator)
			{
				World.Seeds = WorldGenerator->GetDefaultSeeds();
				if (World.IsCreated())
				{
					World.Toggle();
					World.Toggle();
				}
			}
			else
			{
				FVoxelMessages::Error("Can't fill seeds: Invalid World Generator!", &World);
			}
		}));

	bool bIsBPEditor = false;
	bool bIsEditor = false;
	for (auto& Object : Objects)
	{
		bIsBPEditor = Object->GetWorld() == nullptr;
		bIsEditor = !bIsBPEditor && Object->GetWorld()->WorldType == EWorldType::Editor;
	}

	if (!bIsBPEditor && !bIsDataAssetEditor)
	{
		if (bIsEditor)
		{
			FVoxelEditorUtilities::AddButtonToCategory(
				DetailLayout,
				"Voxel - Preview",
				LOCTEXT("Toggle", "Toggle"),
				LOCTEXT("ToggleWorldPreview", "Toggle World Preview"),
				LOCTEXT("Toggle", "Toggle"),
				false,
				CreateWorldsDelegate([](AVoxelWorld& World)
				{
					World.Toggle();
					GEditor->SelectActor(&World, true, true, true, true);
				}));
			FVoxelEditorUtilities::AddButtonToCategory(
				DetailLayout,
				"Voxel - Preview",
				LOCTEXT("Clear", "Clear"),
				LOCTEXT("ClearWorldData", "Clear World Data"),
				LOCTEXT("Clear", "Clear"),
				true,
				CreateWorldsDelegate([](AVoxelWorld& World)
				{
					if (World.IsCreated())
					{
						if (EAppReturnType::Yes == FMessageDialog::Open(
							EAppMsgType::YesNoCancel, 
							LOCTEXT("ClearDataWarning", "This will clear all the voxel world edits! Do you want to continue?")))
						{
							World.GetData().ClearData();
							World.GetLODManager().UpdateBounds(FIntBox::Infinite);
						}
					}
				}));
			FVoxelEditorUtilities::AddButtonToCategory(
				DetailLayout,
				"Voxel - Bake",
				LOCTEXT("Bake", "Bake"),
				LOCTEXT("BakeToStaticMesh", "Bake World To Static Meshes"),
				LOCTEXT("Bake", "Bake"),
				false,
				CreateWorldsDelegate([](AVoxelWorld& World)
				{
					if (World.IsCreated())
					{
						BakeWorld(World);
					}
				}),
				CreateWorldsEnabledDelegate([](AVoxelWorld& World)
				{
					return World.IsCreated();
				}));
		}

		FVoxelEditorUtilities::AddButtonToCategory(
			DetailLayout,
			"Voxel - Save",
			LOCTEXT("Load", "Load"),
			LOCTEXT("LoadFromSave", "Load from Save Object"),
			LOCTEXT("Load", "Load"),
			false,
			CreateWorldsDelegate([](AVoxelWorld& World)
			{
				if (ensure(World.IsCreated()) && ensure(World.SaveObject))
				{
					World.LoadFromSaveObjectEditor();
				}
			}),
			CreateWorldsEnabledDelegate([](AVoxelWorld& World)
			{
				return World.IsCreated() && World.SaveObject;
			}));
		
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, SaveObject));
		FVoxelEditorUtilities::AddPropertyToCategory(
			DetailLayout,
			"Voxel - Save",
			GET_MEMBER_NAME_STATIC(AVoxelWorld, SaveObject),
			false);

		FVoxelEditorUtilities::AddButtonToCategory(
			DetailLayout,
			"Voxel - Save",
			LOCTEXT("SaveFile", "Save File"),
			LOCTEXT("SaveToFile", "Save to File"),
			LOCTEXT("Save", "Save"),
			true,
			CreateWorldsDelegate([](AVoxelWorld& World)
			{
				if (!ensure(World.IsCreated())) return;

				FString FilePath = World.GetDefaultFilePath();
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
				if (!World.SaveToFile(FilePath, Error))
				{
					FMessageDialog::Open(EAppMsgType::Ok, Error);
				}
			}),
			CreateWorldsEnabledDelegate([](AVoxelWorld& World)
			{
				return World.IsCreated();
			}));

		FVoxelEditorUtilities::AddButtonToCategory(
			DetailLayout,
			"Voxel - Save",
			LOCTEXT("LoadFile", "Load File"),
			LOCTEXT("LoadFromFile", "Load from File"),
			LOCTEXT("Load", "Load"),
			true,
			CreateWorldsDelegate([](AVoxelWorld& World)
			{
				if (!ensure(World.IsCreated())) return;

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
					if (!World.LoadFromFile(OutFiles[0], Error))
					{
						FMessageDialog::Open(EAppMsgType::Ok, Error);
					}
				}
			}),
			CreateWorldsEnabledDelegate([](AVoxelWorld& World)
			{
				return World.IsCreated();
			}));
}
}

FReply FVoxelWorldDetails::BakeWorld(AVoxelWorld& World)
{
	FVoxelMessages::ShowVoxelPluginProError("Baking to static mesh requires Voxel Plugin Pro");
	return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE