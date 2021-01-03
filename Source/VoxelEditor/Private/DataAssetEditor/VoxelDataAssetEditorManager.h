// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"

class AVoxelWorld;
class UVoxelDataAsset;
class FPreviewScene;

class FVoxelDataAssetEditorManager : public FGCObject
{
public:
	FVoxelDataAssetEditorManager(UVoxelDataAsset* DataAsset, FPreviewScene& PreviewScene);
	~FVoxelDataAssetEditorManager();

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface

public:
	AVoxelWorld& GetVoxelWorld() const;

public:
	void Save(bool bShowDebug);
	void RecreateWorld();
	
	bool IsDirty() const;

private:
	UVoxelDataAsset* const DataAsset;
	AVoxelWorld* World;

	void CreateWorld();
};