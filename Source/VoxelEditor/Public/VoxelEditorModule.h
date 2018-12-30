// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Toolkits/AssetEditorToolkit.h"

class UVoxelDataAsset;

/** Voxel class editor module interface */
class IVoxelEditorModule : public IModuleInterface
{
public:
};

DECLARE_LOG_CATEGORY_EXTERN(LogVoxelEditor, Verbose, All);
