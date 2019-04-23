// Copyright 2019 Phyronnaz

#include "Details/VoxelEditorDetailsUtils.h"
#include "Misc/MessageDialog.h"
#include "UnrealClient.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "EditorDirectories.h"
#include "Editor/EditorEngine.h"
#include "ObjectTools.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Factories/Factory.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "Voxel"

bool FVoxelEditorDetailsUtils::ShowWarning(const FText& Text)
{
	return FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(LOCTEXT("EditorUtilsWarning", "Warning: {0} \nContinue?"), Text)) == EAppReturnType::Yes;
}

void FVoxelEditorDetailsUtils::ShowError(const FText& Text)
{
	FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("EditorUtilsError", "Error: {0}"), Text));
}

void FVoxelEditorDetailsUtils::EnableRealtime()
{
	FViewport* Viewport = GEditor->GetActiveViewport();
	if (Viewport)
	{
		FViewportClient* Client = Viewport->GetClient();
		if (Client)
		{
#if ENGINE_MINOR_VERSION < 22
			for (FEditorViewportClient* EditorViewportClient : GEditor->AllViewportClients)
#else
			for (FEditorViewportClient* EditorViewportClient : GEditor->GetAllViewportClients())
#endif
			{
				if (EditorViewportClient == Client)
				{
					EditorViewportClient->SetRealtime(true);
					break;
				}
			}
		}
	}
}

void FVoxelEditorDetailsUtils::CreateAsset(UFactory* Factory, UClass* AssetClass, TFunction<bool(UObject*)> CreateAssetFunction)
{
	check(Factory);

	// Create the asset
	UObject* Asset = CreateAssetWithDialog(AssetClass, Factory);

	FNotificationInfo Info{ FText() };
	Info.ExpireDuration = 10.f;
	if (Asset && CreateAssetFunction(Asset))
	{
		Info.Text = FText::Format(LOCTEXT("AssetCreated", "{0} was successfully created"), FText::FromString(Asset->GetPathName()));
		Info.CheckBoxState = ECheckBoxState::Checked;
	}
	else
	{
		Info.Text = FText::Format(LOCTEXT("AssetNotCreated", "Error when creating from {0}!"), Factory->GetDisplayName());
		Info.CheckBoxState = ECheckBoxState::Unchecked;
		if (Asset)
		{
			ObjectTools::DeleteSingleObject(Asset, false);
		}
	}
	FSlateNotificationManager::Get().AddNotification(Info);
}

TSharedRef<STextBlock> FVoxelEditorDetailsUtils::CreateText(const FText& Text)
{
	return SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(Text);
}

TSharedRef<SButton> FVoxelEditorDetailsUtils::CreateButton(
	const FText& Text, 
	const FOnClicked& OnClicked, 
	const TAttribute<bool>& IsEnabled)
{
	return SNew(SButton)
		.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(OnClicked)
		.IsEnabled(IsEnabled)
		[
			CreateText(Text)
		];
}

void FVoxelEditorDetailsUtils::AddButtonToCategory(
	IDetailLayoutBuilder& DetailLayout, 
	const FName& CategoryName, 
	const FText& FilterString, 
	const FText& TextLeftToButton, 
	const FText& ButtonText,
	const FOnClicked& OnClicked, 
	const TAttribute<bool>& IsEnabled)
{
	DetailLayout.EditCategory(CategoryName)
	.AddCustomRow(FilterString)
	.NameContent()
	[
		CreateText(TextLeftToButton)
	]
	.ValueContent()
	.MinDesiredWidth(125.0f)
	.MaxDesiredWidth(125.0f)
	[
		CreateButton(ButtonText, OnClicked, IsEnabled)
	];
}

void FVoxelEditorDetailsUtils::AddPropertyToCategory(
	IDetailLayoutBuilder& DetailLayout, 
	const FName& CategoryName, 
	const FName& PropertyName, 
	UClass* Class)
{
	auto Property = DetailLayout.GetProperty(PropertyName, Class);
	DetailLayout.EditCategory(CategoryName).AddCustomRow(FText::FromString(PropertyName.ToString()))
	.NameContent()
	[
		Property->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(125.0f)
	.MaxDesiredWidth(600.0f)
	[
		Property->CreatePropertyValueWidget()
	];
}

UObject* FVoxelEditorDetailsUtils::CreateAssetWithDialog(
	const FString& AssetName, 
	const FString& PackagePath, 
	UClass* AssetClass, 
	UFactory* Factory, 
	FName CallingContext)
{
	FSaveAssetDialogConfig SaveAssetDialogConfig;
	SaveAssetDialogConfig.DialogTitleOverride = NSLOCTEXT("Voxel", "SaveAssetDialogTitle", "Save Voxel World Data As");
	SaveAssetDialogConfig.DefaultPath = PackagePath;
	SaveAssetDialogConfig.DefaultAssetName = AssetName;
	SaveAssetDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FString SaveObjectPath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);
	if (!SaveObjectPath.IsEmpty())
	{
		FEditorDelegates::OnConfigureNewAssetProperties.Broadcast(Factory);
		if (Factory->ConfigureProperties())
		{
			const FString SavePackageName = FPackageName::ObjectPathToPackageName(SaveObjectPath);
			const FString SavePackagePath = FPaths::GetPath(SavePackageName);
			const FString SaveAssetName = FPaths::GetBaseFilename(SavePackageName);
			FEditorDirectories::Get().SetLastDirectory(ELastDirectory::NEW_ASSET, PackagePath);

			return FAssetToolsModule::GetModule().Get().CreateAsset(SaveAssetName, SavePackagePath, AssetClass, Factory, CallingContext);
		}
	}

	return nullptr;
}

UObject* FVoxelEditorDetailsUtils::CreateAssetWithDialog(
	UClass* AssetClass,
	UFactory* Factory,
	FName CallingContext)
{
	if (Factory != nullptr)
	{
		// Determine the starting path. Try to use the most recently used directory
		FString AssetPath;

		const FString DefaultFilesystemDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::NEW_ASSET);
		if (DefaultFilesystemDirectory.IsEmpty() || !FPackageName::TryConvertFilenameToLongPackageName(DefaultFilesystemDirectory, AssetPath))
		{
			// No saved path, just use the game content root
			AssetPath = TEXT("/Game");
		}

		FString PackageName;
		FString AssetName;
		FAssetToolsModule::GetModule().Get().CreateUniqueAssetName(AssetPath / Factory->GetDefaultNewAssetName(), TEXT(""), PackageName, AssetName);

		return CreateAssetWithDialog(AssetName, AssetPath, AssetClass, Factory, CallingContext);
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE