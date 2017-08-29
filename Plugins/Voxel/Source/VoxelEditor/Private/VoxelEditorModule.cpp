// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Containers/Array.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "Toolkits/AssetEditorToolkit.h"

#include "AssetTools/VoxelActions.h"
#include "Styles/VoxelEditorStyle.h"
#include "VoxelEditorSettings.h"
#include "PropertyEditorModule.h"
#include "VoxelWorldDetails.h"
#include "VoxelAssetDetails.h"


#define LOCTEXT_NAMESPACE "FVoxelEditorModule"


/**
 * Implements the VoxelEditor module.
 */
class FVoxelEditorModule
	: public IHasMenuExtensibility
	, public IHasToolBarExtensibility
	, public IModuleInterface
{
public:

	//~ IHasMenuExtensibility interface

	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override
	{
		return MenuExtensibilityManager;
	}

public:

	//~ IHasToolBarExtensibility interface

	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override
	{
		return ToolBarExtensibilityManager;
	}

public:

	//~ IModuleInterface interface

	virtual void StartupModule() override
	{
		Style = MakeShareable(new FVoxelEditorStyle());

		//		FVoxelEditorCommands::Register();

		RegisterAssetTools();
		RegisterMenuExtensions();
		RegisterSettings();
		RegisterClassLayout();
	}

	virtual void ShutdownModule() override
	{
		UnregisterAssetTools();
		UnregisterMenuExtensions();
		UnregisterSettings();
		UnregisterClassLayout();
	}

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}

protected:

	/** Registers asset tool actions. */
	void RegisterAssetTools()
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		RegisterAssetTypeAction(AssetTools, MakeShareable(new FVoxelActions(Style.ToSharedRef())));
	}

	/**
	 * Registers a single asset type action.
	 *
	 * @param AssetTools The asset tools object to register with.
	 * @param Action The asset type action to register.
	 */
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		RegisteredAssetTypeActions.Add(Action);
	}

	/** Register the text asset editor settings. */
	void RegisterSettings()
	{
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Editor", "Plugins", "Voxel",
																				   LOCTEXT("VoxelSettingsName", "Text Asset"),
																				   LOCTEXT("VoxelSettingsDescription", "Configure the Text Asset plug-in."),
																				   GetMutableDefault<UVoxelEditorSettings>()
			);
		}
	}

	void RegisterClassLayout()
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		//Custom detail views
		PropertyModule.RegisterCustomClassLayout("VoxelWorld", FOnGetDetailCustomizationInstance::CreateStatic(&FVoxelWorldDetails::MakeInstance));
		PropertyModule.RegisterCustomClassLayout("VoxelAsset", FOnGetDetailCustomizationInstance::CreateStatic(&FVoxelAssetDetails::MakeInstance));
	}

	/** Unregisters asset tool actions. */
	void UnregisterAssetTools()
	{
		FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");

		if (AssetToolsModule != nullptr)
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();

			for (auto Action : RegisteredAssetTypeActions)
			{
				AssetTools.UnregisterAssetTypeActions(Action);
			}
		}
	}

	/** Unregister the text asset editor settings. */
	void UnregisterSettings()
	{
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->UnregisterSettings("Editor", "Plugins", "Voxel");
		}
	}

	void UnregisterClassLayout()
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");

		if (PropertyModule != nullptr)
		{
			PropertyModule->UnregisterCustomClassLayout("VoxelWorld");
			PropertyModule->UnregisterCustomClassLayout("VoxelAsset");
		}
	}

protected:

	/** Registers main menu and tool bar menu extensions. */
	void RegisterMenuExtensions()
	{
		MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
		ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);
	}

	/** Unregisters main menu and tool bar menu extensions. */
	void UnregisterMenuExtensions()
	{
		MenuExtensibilityManager.Reset();
		ToolBarExtensibilityManager.Reset();
	}

private:

	/** Holds the menu extensibility manager. */
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;

	/** The collection of registered asset type actions. */
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;

	/** Holds the plug-ins style set. */
	TSharedPtr<ISlateStyle> Style;

	/** Holds the tool bar extensibility manager. */
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;
};


IMPLEMENT_MODULE(FVoxelEditorModule, VoxelEditor);


#undef LOCTEXT_NAMESPACE
