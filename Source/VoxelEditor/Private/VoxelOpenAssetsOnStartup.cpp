// Copyright 2020 Phyronnaz

#include "VoxelOpenAssetsOnStartup.h"
#include "VoxelMinimal.h"
#include "VoxelUtilities/VoxelConfigUtilities.h"

#include "Editor.h"
#include "Engine/World.h"
#include "GameMapsSettings.h"
#include "Containers/Ticker.h"
#include "ContentBrowserModule.h"
#include "Toolkits/AssetEditorManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

void UVoxelOpenAssetsOnStartup::Init()
{
	FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([=](float)
	{
		GetMutableDefault<UVoxelOpenAssetsOnStartup>()->ActualInit();
		return false;
	}));
}

void UVoxelOpenAssetsOnStartup::ActualInit()
{
	FVoxelConfigUtilities::LoadConfig(this, "OpenAssetsOnStartup");
	
	if (bEnableOpenAssetsOnStartup)
	{
		for (auto& It : AssetsToOpenOnStartup)
		{
			if (!It.Value)
			{
				continue;
			}

#if ENGINE_MINOR_VERSION < 24
			FAssetEditorManager::Get().OpenEditorForAsset(It.Key.ToString());
#else
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(It.Key.ToString());
#endif
		}
	}
	
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Add(FContentBrowserMenuExtender_SelectedAssets::CreateLambda([=](const TArray<FAssetData>& SelectedAssets)
	{
		const auto Extender = MakeShared<FExtender>();
		
		if (bEnableOpenAssetsOnStartup && SelectedAssets.Num() == 1)
		{
			const auto Asset = SelectedAssets[0];
			const FString Path = Asset.PackagePath.ToString() / Asset.AssetName.ToString();

			Extender->AddMenuExtension(
				"CommonAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateLambda([=](FMenuBuilder& MenuBuilder)
				{
					MenuBuilder.AddMenuEntry(
					TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([=]()
					{
						return AssetsToOpenOnStartup.FindRef(*Path) ? VOXEL_LOCTEXT("Stop opening on startup") : VOXEL_LOCTEXT("Open on startup");
					})),
					TAttribute<FText>(),
					FSlateIcon(NAME_None, NAME_None),
					FUIAction(FExecuteAction::CreateLambda([=]()
					{
						bool& bValue = AssetsToOpenOnStartup.FindOrAdd(*Path);
						bValue = !bValue;
						FVoxelConfigUtilities::SaveConfig(this, "OpenAssetsOnStartup");
					})));
				}));
		}
		
		if (bShowSetAsStartupMap &&
			SelectedAssets.Num() == 1 && 
			SelectedAssets[0].GetClass() == UWorld::StaticClass() &&
			GetDefault<UGameMapsSettings>()->EditorStartupMap != SelectedAssets[0].ToSoftObjectPath())
		{
			const auto Asset = SelectedAssets[0];

			Extender->AddMenuExtension(
				"CommonAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateLambda([=](FMenuBuilder& MenuBuilder)
				{
					MenuBuilder.AddMenuEntry(
					VOXEL_LOCTEXT("Set as editor startup map"),
					TAttribute<FText>(),
					FSlateIcon(NAME_None, NAME_None),
					FUIAction(FExecuteAction::CreateLambda([=]()
					{
						auto* Settings = GetMutableDefault<UGameMapsSettings>();
						Settings->EditorStartupMap = SelectedAssets[0].ToSoftObjectPath();

						auto* Property = UE_25_SWITCH(FindField, FindFProperty)<FProperty>(UGameMapsSettings::StaticClass(), GET_MEMBER_NAME_CHECKED(UGameMapsSettings, EditorStartupMap));
						Settings->UpdateSinglePropertyInConfigFile(Property, Settings->GetDefaultConfigFilename());
					})));
				}));
		}

		return Extender;
	}));
}