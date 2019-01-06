// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Toolkits/AssetEditorToolkit.h"

class UVoxelDataAsset;
class UVoxelMaterialCollection;

/** Voxel class editor module interface */
class IVoxelEditorModule : public IModuleInterface
{
public:

	virtual bool GenerateSingleMaterials(UVoxelMaterialCollection* Collection, FString& OutError) = 0;
	virtual bool GenerateDoubleMaterials(UVoxelMaterialCollection* Collection, FString& OutError) = 0;
	virtual bool GenerateTripleMaterials(UVoxelMaterialCollection* Collection, FString& OutError) = 0;
};

DECLARE_LOG_CATEGORY_EXTERN(LogVoxelEditor, Verbose, All);
