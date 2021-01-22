// Copyright 2021 Phyronnaz

#include "Factories/VoxelDataAssetFromMagicaVoxFactory.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssets/VoxelDataAssetData.h"
#include "VoxelImporters/VoxelMagicaVoxImporter.h"

#include "Editor.h"
#include "AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "Framework/SlateDelegates.h"
#include "Misc/FeedbackContext.h"

UVoxelDataAssetFromMagicaVoxFactory::UVoxelDataAssetFromMagicaVoxFactory()
{
	bEditorImport = true;
	SupportedClass = UVoxelDataAsset::StaticClass();
	Formats.Add(TEXT("vox;Magica Voxel Asset"));
}

bool UVoxelDataAssetFromMagicaVoxFactory::ConfigureProperties()
{
	// Load from default
	bUsePalette = GetDefault<UVoxelDataAssetFromMagicaVoxFactory>()->bUsePalette;

	TSharedRef<SWindow> PickerWindow = SNew(SWindow)
		.Title(VOXEL_LOCTEXT("Import Magica Vox"))
		.SizingRule(ESizingRule::Autosized);

	bool bSuccess = false;

	auto OnOkClicked = FOnClicked::CreateLambda([&]() 
	{
		bSuccess = true;
		PickerWindow->RequestDestroyWindow();
		return FReply::Handled();
	});
	auto OnCancelClicked = FOnClicked::CreateLambda([&]() 
	{
		bSuccess = false;
		PickerWindow->RequestDestroyWindow();
		return FReply::Handled();
	});
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, false, FDetailsViewArgs::HideNameArea);

	auto DetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsPanel->SetObject(this);

	auto Widget =
		SNew(SBorder)
		.Visibility(EVisibility::Visible)
		.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
		[
			SNew(SBox)
			.Visibility(EVisibility::Visible)
			.WidthOverride(520.0f)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					DetailsPanel
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				.Padding(8)
				[
					SNew(SUniformGridPanel)
					.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.Text(VOXEL_LOCTEXT("Create"))
						.HAlign(HAlign_Center)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.OnClicked(OnOkClicked)
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
						.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
					]
					+SUniformGridPanel::Slot(1,0)
					[
						SNew(SButton)
						.Text(VOXEL_LOCTEXT("Cancel"))
						.HAlign(HAlign_Center)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.OnClicked(OnCancelClicked)
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
						.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
					]
				]
			]
		];

	PickerWindow->SetContent(Widget);

	GEditor->EditorAddModalWindow(PickerWindow);

	// Save to default
	GetMutableDefault<UVoxelDataAssetFromMagicaVoxFactory>()->bUsePalette = bUsePalette;

	return bSuccess;
}

bool UVoxelDataAssetFromMagicaVoxFactory::FactoryCanImport(const FString& Filename)
{
	return FPaths::GetExtension(Filename) == TEXT("vox");
}

UObject* UVoxelDataAssetFromMagicaVoxFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	ensure(Flags & RF_Transactional);

	FString Error;
	const auto Scene = FVoxelMagicaVoxScene::LoadScene(Filename, Error);
	if (!Scene)
	{
		Warn->Logf(ELogVerbosity::Error, TEXT("Failed to load scene: %s"), *Error);
		return nullptr;
	}

	TArray<UVoxelDataAsset*> Assets;
	auto* SceneAsset = Scene->Import(InParent, InName, Flags, bUsePalette, Assets);
	if (!SceneAsset)
	{
		return nullptr;
	}

	SceneAsset->ImportPath = Filename;
	for (int32 ModelIndex = 0; ModelIndex < Assets.Num(); ModelIndex++)
	{
		auto* Asset = Assets[ModelIndex];
		if (!Asset)
		{
			continue;
		}

		Asset->Paths = { Filename };
		Asset->Source = EVoxelDataAssetImportSource::MagicaVox;
		Asset->ImportSettings_MagicaVox.bUsePalette = bUsePalette;
		Asset->ImportSettings_MagicaVox.ModelIndex = ModelIndex;

		FAssetRegistryModule::AssetCreated(Asset);
		Asset->MarkPackageDirty();
	}
	
	return SceneAsset;
}

bool UVoxelDataAssetFromMagicaVoxFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	if (auto* Asset = Cast<UVoxelDataAsset>(Obj))
	{
		if (Asset->Source == EVoxelDataAssetImportSource::MagicaVox)
		{
			OutFilenames = Asset->Paths;
			OutFilenames.SetNum(1);
			return true;
		}
	}
	if (auto* Scene = Cast<UVoxelMagicaVoxScene>(Obj))
	{
		return true;
	}
	return false;
}

void UVoxelDataAssetFromMagicaVoxFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	if (auto* Asset = Cast<UVoxelDataAsset>(Obj))
	{
		Asset->Paths = NewReimportPaths;
		ensure(Asset->Paths.Num() == 1);
	}
	if (auto* Scene = Cast<UVoxelMagicaVoxScene>(Obj))
	{
		if (ensure(NewReimportPaths.Num() == 1))
		{
			Scene->ImportPath = NewReimportPaths[0];
		}
	}
}

EReimportResult::Type UVoxelDataAssetFromMagicaVoxFactory::Reimport(UObject* Obj)
{
	auto* Asset = Cast<UVoxelDataAsset>(Obj);
	if (!Asset)
	{
		return EReimportResult::Failed;
	}

	FString Error;
	const auto Scene = FVoxelMagicaVoxScene::LoadScene(Asset->Paths[0], Error);
	if (!Scene)
	{
		return EReimportResult::Failed;
	}

	FVoxelDataAssetImportSettings_MagicaVox& ImportSettings = Asset->ImportSettings_MagicaVox;
	bUsePalette = ImportSettings.bUsePalette;
	ConfigureProperties();
	ImportSettings.bUsePalette = bUsePalette;
		
	const auto Data = MakeVoxelShared<FVoxelDataAssetData>();
	if (!Scene->ImportModel(*Data, ImportSettings.ModelIndex, ImportSettings.bUsePalette))
	{
		return EReimportResult::Failed;
	}
	Asset->SetData(Data);
	
	return EReimportResult::Succeeded;
}

int32 UVoxelDataAssetFromMagicaVoxFactory::GetPriority() const
{
	return ImportPriority;
}