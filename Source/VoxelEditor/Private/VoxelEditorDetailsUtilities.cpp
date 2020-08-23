// Copyright 2020 Phyronnaz

#include "VoxelEditorDetailsUtilities.h"
#include "VoxelMinimal.h"

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

bool FVoxelEditorUtilities::ShowWarning(const FText& Text)
{
	return FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(VOXEL_LOCTEXT("Warning: {0} \nContinue?"), Text)) == EAppReturnType::Yes;
}

void FVoxelEditorUtilities::ShowError(const FText& Text)
{
	FMessageDialog::Open(EAppMsgType::Ok, FText::Format(VOXEL_LOCTEXT("Error: {0}"), Text));
}

void FVoxelEditorUtilities::EnableRealtime()
{
	FViewport* Viewport = GEditor->GetActiveViewport();
	if (Viewport)
	{
		FViewportClient* Client = Viewport->GetClient();
		if (Client)
		{
			for (FEditorViewportClient* EditorViewportClient : GEditor->GetAllViewportClients())
			{
				if (EditorViewportClient == Client)
				{
					EditorViewportClient->SetRealtime(true);
					EditorViewportClient->SetShowStats(true); // Show stats as well
					break;
				}
			}
		}
	}
}

TSharedRef<STextBlock> FVoxelEditorUtilities::CreateText(const FText& Text, TAttribute<FSlateColor> ColorAndOpacity)
{
	return SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(Text)
		.ColorAndOpacity(ColorAndOpacity);
}

TSharedRef<SButton> FVoxelEditorUtilities::CreateButton(
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

void FVoxelEditorUtilities::AddButtonToCategory(
	IDetailLayoutBuilder& DetailLayout, 
	const FName& CategoryName, 
	const FText& FilterString, 
	const FText& TextLeftToButton, 
	const FText& ButtonText,
	bool bForAdvanced,
	const FOnClicked& OnClicked, 
	const TAttribute<bool>& IsEnabled)
{
	DetailLayout.EditCategory(CategoryName)
	.AddCustomRow(FilterString, bForAdvanced)
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

void FVoxelEditorUtilities::AddPropertyToCategory(
	IDetailLayoutBuilder& DetailLayout, 
	const FName& CategoryName, 
	const FName& PropertyName, 
	bool bForAdvanced,
	UClass* Class)
{
	const auto Property = DetailLayout.GetProperty(PropertyName, Class);
	DetailLayout.EditCategory(CategoryName).AddProperty(Property, bForAdvanced ? EPropertyLocation::Advanced : EPropertyLocation::Default);
}

UObject* FVoxelEditorUtilities::CreateAssetWithDialog(
	const FString& AssetName, 
	const FString& PackagePath, 
	UClass* AssetClass, 
	UFactory* Factory, 
	FName CallingContext)
{
	FSaveAssetDialogConfig SaveAssetDialogConfig;
	SaveAssetDialogConfig.DialogTitleOverride = VOXEL_LOCTEXT("Save As");
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

UObject* FVoxelEditorUtilities::CreateAssetWithDialog(
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