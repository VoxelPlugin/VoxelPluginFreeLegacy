// Copyright 2020 Phyronnaz

#include "Factories/VoxelDataAssetFromMagicaVoxFactory.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssets/VoxelDataAssetData.h"

#include "Editor.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "Framework/SlateDelegates.h"

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
	FVoxelMessages::Info("Importing MagicaVox assets requires Voxel Plugin Pro");
	return nullptr;
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
	return false;
}

void UVoxelDataAssetFromMagicaVoxFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	if (auto* Asset = Cast<UVoxelDataAsset>(Obj))
	{
		Asset->Paths = NewReimportPaths;
		ensure(Asset->Paths.Num() == 1);
	}
}

EReimportResult::Type UVoxelDataAssetFromMagicaVoxFactory::Reimport(UObject* Obj)
{
	FVoxelMessages::Info("Converting meshes to voxels requires Voxel Plugin Pro");
	return EReimportResult::Failed;
}

int32 UVoxelDataAssetFromMagicaVoxFactory::GetPriority() const
{
	return ImportPriority;
}