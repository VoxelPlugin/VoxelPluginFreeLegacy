// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"

class FAdvancedPreviewScene;
class FVoxelEditorToolsPanel;
class AVoxelWorld;
class UVoxelDataAsset;

class IVoxelDataAssetEditor : public FAssetEditorToolkit
{
public:
	virtual FAdvancedPreviewScene& GetPreviewScene() const = 0;
	virtual AVoxelWorld& GetVoxelWorld() const = 0;
	virtual UVoxelDataAsset& GetDataAsset() const = 0;
	virtual FVoxelEditorToolsPanel& GetPanel() const = 0;
};