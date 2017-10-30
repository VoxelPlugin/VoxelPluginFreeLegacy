// Copyright 2017 Phyronnaz

#include "Containers/Array.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "Toolkits/AssetEditorToolkit.h"

#include "PropertyEditorModule.h"

#include "VoxelWorldDetails.h"
#include "VoxelMeshAssetDetails.h"
#include "VoxelLandscapeImporterDetails.h"
#include "VoxelSplineImporterDetails.h"

#include "IAssetTools.h"
#include "AssetToolsModule.h"

#include "AssetTools/AssetTypeActions_VoxelGrassType.h"

#define LOCTEXT_NAMESPACE "FVoxelEditorModule"

/**
 * Implements the VoxelEditor module.
 */
class FVoxelEditorModule : public IModuleInterface
{
public:

	//~ IModuleInterface interface

	virtual void StartupModule() override
	{
		RegisterClassLayout();
		RegisterAssetTools();
	}

	virtual void ShutdownModule() override
	{
		UnregisterClassLayout();
		UnregisterAssetTools();
	}

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}


protected:

	void RegisterClassLayout()
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		//Custom detail views
		PropertyModule.RegisterCustomClassLayout("VoxelWorld", FOnGetDetailCustomizationInstance::CreateStatic(&FVoxelWorldDetails::MakeInstance));
		PropertyModule.RegisterCustomClassLayout("VoxelLandscapeImporter", FOnGetDetailCustomizationInstance::CreateStatic(&UVoxelLandscapeImporterDetails::MakeInstance));
		PropertyModule.RegisterCustomClassLayout("VoxelSplineImporter", FOnGetDetailCustomizationInstance::CreateStatic(&UVoxelSplineImporterDetails::MakeInstance));
		PropertyModule.RegisterCustomClassLayout("VoxelMeshAsset", FOnGetDetailCustomizationInstance::CreateStatic(&FVoxelMeshAssetDetails::MakeInstance));
		PropertyModule.NotifyCustomizationModuleChanged();
	}

	void UnregisterClassLayout()
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");

		if (PropertyModule != nullptr)
		{
			PropertyModule->UnregisterCustomClassLayout("VoxelWorld");
			PropertyModule->UnregisterCustomClassLayout("VoxelLandscapeImporter");
			PropertyModule->UnregisterCustomClassLayout("VoxelSplineImporter");
			PropertyModule->UnregisterCustomClassLayout("VoxelMeshAsset");
			PropertyModule->NotifyCustomizationModuleChanged();
		}
	}

	/** Registers asset tool actions. */
	void RegisterAssetTools()
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_VoxelGrassType));
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

private:
	/** The collection of registered asset type actions. */
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
};


IMPLEMENT_MODULE(FVoxelEditorModule, VoxelEditor);


#undef LOCTEXT_NAMESPACE
