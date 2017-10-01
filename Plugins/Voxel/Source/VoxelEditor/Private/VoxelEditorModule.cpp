// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

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
#include "LandscapeVoxelAssetDetails.h"


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
	}

	virtual void ShutdownModule() override
	{
		UnregisterClassLayout();
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
		PropertyModule.RegisterCustomClassLayout("LandscapeVoxelAsset", FOnGetDetailCustomizationInstance::CreateStatic(&ULandscapeVoxelAssetDetails::MakeInstance));
		PropertyModule.RegisterCustomClassLayout("VoxelMeshAsset", FOnGetDetailCustomizationInstance::CreateStatic(&FVoxelMeshAssetDetails::MakeInstance));
		PropertyModule.NotifyCustomizationModuleChanged();
	}

	void UnregisterClassLayout()
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");

		if (PropertyModule != nullptr)
		{
			PropertyModule->UnregisterCustomClassLayout("VoxelWorld");
			PropertyModule->UnregisterCustomClassLayout("LandscapeVoxelAsset");
			PropertyModule->UnregisterCustomClassLayout("VoxelMeshAsset");
			PropertyModule->NotifyCustomizationModuleChanged();
		}
	}
};


IMPLEMENT_MODULE(FVoxelEditorModule, VoxelEditor);


#undef LOCTEXT_NAMESPACE
