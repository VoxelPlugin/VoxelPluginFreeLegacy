// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelConfigEnums.h"

class IVoxelRenderer;
class FVoxelPolygonizerAsyncWork;
class FVoxelData;
class FVoxelQueuedThreadPool;
struct FIntBox;
struct FVoxelRendererSettings;

class VOXEL_API FVoxelRenderFactory
{
public:
	static TSharedRef<IVoxelRenderer> GetVoxelRenderer(EVoxelRenderType RenderType, const FVoxelRendererSettings& Settings);
};