// Copyright 2020 Phyronnaz

#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelData/VoxelData.h"
#include "VoxelWorld.h"

FVoxelLODSettings::FVoxelLODSettings(
	const AVoxelWorld* InWorld,
	EVoxelPlayType PlayType,
	const TVoxelSharedRef<IVoxelRenderer>& Renderer,
	const TVoxelSharedRef<IVoxelPool>& Pool,
	const FVoxelData* Data)
	: Renderer(Renderer)
	, Pool(Pool)
	, VoxelSize(InWorld->VoxelSize)
	, OctreeDepth(Data
		? FVoxelUtilities::GetChunkDepthFromDataDepth(Data->Depth)
		: FVoxelUtilities::ClampChunkDepth(InWorld->OctreeDepth))
	, WorldBounds(Data
		? Data->WorldBounds
		: InWorld->GetWorldBounds())
	, bConstantLOD(PlayType == EVoxelPlayType::Game
		? InWorld->bConstantLOD
		: false)
	, bStaticWorld(PlayType == EVoxelPlayType::Game
		? InWorld->bStaticWorld
		: false)
	, MinDelayBetweenLODUpdates(InWorld->MinDelayBetweenLODUpdates)
	, bEnableTransitions(InWorld->bEnableTransitions)
	, bInvertTransitions(InWorld->RenderType == EVoxelRenderType::SurfaceNets)

	, World(InWorld->GetWorld())
{
}