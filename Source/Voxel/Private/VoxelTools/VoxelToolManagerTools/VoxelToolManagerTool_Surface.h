// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTexture.h"
#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerTool.h"

class FVoxelToolManagerTool_Surface : public FVoxelToolManagerTool
{
public:
	const FVoxelToolManager_SurfaceSettings& SurfaceSettings;
	
	explicit FVoxelToolManagerTool_Surface(const UVoxelToolManager& ToolManager);
	
	//~ Begin FVoxelToolManagerTool Interface
	virtual void Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData) override;
	//~ End FVoxelToolManagerTool Interface

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface

private:
	struct FMaskWorldGeneratorCache
	{
		UObject* GeneratorObject = nullptr;
		float Scale = 0;
		float Radius = 0;
		bool bScaleWithRadius = false;

		TMap<FName, int32> Seeds;
		TVoxelTexture<float> Texture;
		UTexture2D* RenderTexture = nullptr;
	};
	FMaskWorldGeneratorCache MaskWorldGeneratorCache;
};