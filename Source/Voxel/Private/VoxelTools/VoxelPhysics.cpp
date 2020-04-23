// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelPhysics.h"
#include "VoxelTools/VoxelPhysicsPartSpawner.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelDataAccelerator.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelWorldGenerators/VoxelEmptyWorldGenerator.h"
#include "VoxelDebug/VoxelDebugUtilities.h"

#include "VoxelWorld.h"
#include "VoxelIntVectorUtilities.h"

#include "Async/Async.h"
#include "DrawDebugHelpers.h"


void UVoxelPhysicsTools::ApplyVoxelPhysics(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FIntBox Bounds,
	TScriptInterface<IVoxelPhysicsPartSpawner> PartSpawner,
	int32 MinParts,
	bool bDebug,
	bool bHideLatentWarnings)
{
	VOXEL_PRO_ONLY_VOID();
}