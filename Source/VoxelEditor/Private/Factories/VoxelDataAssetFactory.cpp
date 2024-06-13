// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "Factories/VoxelDataAssetFactory.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssets/VoxelDataAssetData.inl"
#include "VoxelImporters/VoxelMeshImporter.h"
#include "VoxelFeedbackContext.h"
#include "VoxelMessages.h"

#include "Editor.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Engine/StaticMesh.h"
#include "Framework/Notifications/NotificationManager.h"

#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Notifications/SNotificationList.h"

UVoxelDataAssetFactory::UVoxelDataAssetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = true;
	SupportedClass = UVoxelDataAsset::StaticClass();
}

UObject* UVoxelDataAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto* NewDataAsset = NewObject<UVoxelDataAsset>(InParent, Class, Name, Flags | RF_Transactional);

	auto Data = MakeVoxelShared<FVoxelDataAssetData>();
	Data->SetSize(FIntVector(1, 1, 3), false);
	Data->SetValue(0, 0, 0, FVoxelValue::Full());
	Data->SetValue(0, 0, 1, FVoxelValue::Empty());
	Data->SetValue(0, 0, 2, FVoxelValue::Full());
	NewDataAsset->SetData(Data);

	return NewDataAsset;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelDataAssetFromMeshImporterFactory::UVoxelDataAssetFromMeshImporterFactory()
{
	bEditAfterNew = true;
	bEditorImport = true;
	SupportedClass = UVoxelDataAsset::StaticClass();
}

UObject* UVoxelDataAssetFromMeshImporterFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	FVoxelMessages::Info("Converting meshes to voxels requires Voxel Plugin Pro");
	return nullptr;
}

FString UVoxelDataAssetFromMeshImporterFactory::GetDefaultNewAssetName() const
{
	return MeshImporter->StaticMesh->GetName();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	auto DetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsPanel->SetObject(this);

	auto Widget =
		SNew(SBorder)
		.Visibility(EVisibility::Visible)
		.BorderImage(FAppStyle::GetBrush("Menu.Background"))
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
					.SlotPadding(FAppStyle::GetMargin("StandardDialog.SlotPadding"))
					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.Text(VOXEL_LOCTEXT("Create"))
						.HAlign(HAlign_Center)
						.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
						.OnClicked(OnOkClicked)
						.ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
						.TextStyle(FAppStyle::Get(), "FlatButton.DefaultTextStyle")
					]
					+SUniformGridPanel::Slot(1,0)
					[
						SNew(SButton)
						.Text(VOXEL_LOCTEXT("Cancel"))
						.HAlign(HAlign_Center)
						.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
						.OnClicked(OnCancelClicked)
						.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
						.TextStyle(FAppStyle::Get(), "FlatButton.DefaultTextStyle")
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelDataAssetFromRawVoxFactory::UVoxelDataAssetFromRawVoxFactory()
{
	bEditorImport = true;
	SupportedClass = UVoxelDataAsset::StaticClass();
	Formats.Add(TEXT("rawvox;3D Coat RawVox"));
}

bool UVoxelDataAssetFromRawVoxFactory::FactoryCanImport(const FString& Filename)
{
	return FPaths::GetExtension(Filename) == TEXT("rawvox");
}

UObject* UVoxelDataAssetFromRawVoxFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	FVoxelMessages::Info("Importing RawVox assets requires Voxel Plugin Pro");
	return nullptr;
}

bool UVoxelDataAssetFromRawVoxFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	if (auto* Asset = Cast<UVoxelDataAsset>(Obj))
	{
		if (Asset->Source == EVoxelDataAssetImportSource::RawVox)
		{
			OutFilenames = Asset->Paths;
			return true;
		}
	}
	return false;
}

void UVoxelDataAssetFromRawVoxFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	if (auto* Asset = Cast<UVoxelDataAsset>(Obj))
	{
		Asset->Paths = NewReimportPaths;
	}
}

EReimportResult::Type UVoxelDataAssetFromRawVoxFactory::Reimport(UObject* Obj)
{
	FVoxelMessages::Info("Importing RawVox assets requires Voxel Plugin Pro");
	return EReimportResult::Failed;
}

int32 UVoxelDataAssetFromRawVoxFactory::GetPriority() const
{
	return ImportPriority;
}