// Copyright 2020 Phyronnaz

#include "VoxelVDBFactory.h"
#include "VoxelMessages.h"

#include "Misc/Paths.h"

#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"

#include "Editor.h"
#include "EditorStyleSet.h"
#include "PropertyEditorModule.h"
#include "DetailLayoutBuilder.h"

#include "Modules/ModuleManager.h"

UVoxelVDBFactory::UVoxelVDBFactory()
{	
	bEditorImport = true;
	SupportedClass = UVoxelVDBAsset::StaticClass();
	Formats.Add(TEXT("vdb;OpenVDB file"));
}

bool UVoxelVDBFactory::FactoryCanImport(const FString& Filename)
{
	return FPaths::GetExtension(Filename) == TEXT("vdb");
}

UObject* UVoxelVDBFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	auto* NewAsset = NewObject<UVoxelVDBAsset>(InParent, InName, Flags | RF_Transactional);
	if (DoImport(*NewAsset, Filename, {}))
	{
		return NewAsset;
	}
	else
	{
		return nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelVDBFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	if (auto* Asset = Cast<UVoxelVDBAsset>(Obj))
{
		OutFilenames = { Asset->ImportPath };
		return true;
	}
	return false;
}

void UVoxelVDBFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	auto* Asset = Cast<UVoxelVDBAsset>(Obj);
	if (Asset && NewReimportPaths.Num() > 0)
	{
		Asset->ImportPath = NewReimportPaths[0];
	}
}	

EReimportResult::Type UVoxelVDBFactory::Reimport(UObject* Obj)
{
	if (auto* Asset = Cast<UVoxelVDBAsset>(Obj))
	{
		if (DoImport(*Asset, Asset->ImportPath, Asset->ChannelConfigs))
		{
			return EReimportResult::Succeeded;
		}
		else
		{
			return EReimportResult::Failed;
		}
	}
	return EReimportResult::Failed;
}

int32 UVoxelVDBFactory::GetPriority() const
{
	return ImportPriority;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelVDBFactory::DoImport(UVoxelVDBAsset& Asset, const FString& Path, const TMap<FName, FVoxelVDBImportChannelConfig>& ExistingChannelConfigs)
{
	const auto Data = Asset.MakeData();

	FString Error;

	const auto GetChannelConfigs = [&](TMap<FName, FVoxelVDBImportChannelConfig>& ChannelConfigs)
	{
		SetFlags(RF_Transactional);

		Mappings.Reset();
		for (auto& It : ChannelConfigs)
		{
			auto Value = It.Value;
			if (auto* ExistingValue = ExistingChannelConfigs.Find(It.Key))
			{
				Value = *ExistingValue;
			}
			Mappings.Add({ It.Key, Value });
		}
		
		TSharedRef<SWindow> PickerWindow = SNew(SWindow)
			.Title(VOXEL_LOCTEXT("Import Heightmap"))
			.SizingRule(ESizingRule::UserSized)
			.ClientSize(FVector2D(600, 400));

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
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							DetailsPanel
						]
					]
					+ SVerticalBox::Slot()
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
						+ SUniformGridPanel::Slot(1,0)
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

		for (auto& Mapping : Mappings)
		{
			if (auto* Value = ChannelConfigs.Find(Mapping.ChannelName))
			{
				*Value = Mapping.ChannelConfig;
			}
		}
		
		Asset.ImportPath = Path;
		Asset.ChannelConfigs = ChannelConfigs;
		
		return bSuccess;
	};
	
	if (!Data->LoadVDB(Path, Error, GetChannelConfigs))
	{
		FVoxelMessages::Error("Failed to import VDB: " + Error);
		return false;
	}

	Asset.SetData(Data);
	return true;
}