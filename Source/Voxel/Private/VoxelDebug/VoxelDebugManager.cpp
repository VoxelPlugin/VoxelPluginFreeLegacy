// Copyright 2020 Phyronnaz

#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelDebug/VoxelDebugUtilities.h"
#include "VoxelData/VoxelData.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelData/VoxelDataIncludes.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelTools/VoxelDataTools.h"
#include "VoxelTools/VoxelSurfaceTools.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelMessages.h"
#include "VoxelWorld.h"
#include "IVoxelPool.h"
#include "VoxelThreadPool.h"

#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

static TAutoConsoleVariable<int32> CVarShowUpdatedChunks(
	TEXT("voxel.renderer.ShowUpdatedChunks"),
	0,
	TEXT("If true, will show the chunks recently updated"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowRenderChunks(
	TEXT("voxel.renderer.ShowRenderChunks"),
	0,
	TEXT("If true, will show the render chunks"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowMultiplayerSyncedChunks(
	TEXT("voxel.multiplayer.ShowSyncedChunks"),
	0,
	TEXT("If true, will show the synced chunks"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowValuesState(
	TEXT("voxel.data.ShowValuesState"),
	0,
	TEXT("If true, will show the values data chunks and their status (cached/created...)"),
	ECVF_Default);
static TAutoConsoleVariable<int32> CVarShowMaterialsState(
	TEXT("voxel.data.ShowMaterialsState"),
	0,
	TEXT("If true, will show the materials data chunks and their status (cached/created...)"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowDirtyValues(
	TEXT("voxel.data.ShowDirtyValues"),
	0,
	TEXT("If true, will show the data chunks with dirty values"),
	ECVF_Default);
static TAutoConsoleVariable<int32> CVarShowDirtyMaterials(
	TEXT("voxel.data.ShowDirtyMaterials"),
	0,
	TEXT("If true, will show the data chunks with dirty materials"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarFreezeDebug(
	TEXT("voxel.FreezeDebug"),
	0,
	TEXT("If true, won't clear previous frames boxes"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarDebugDrawTime(
	TEXT("voxel.debug.DrawTime"),
	1,
	TEXT("Draw time will be multiplied by this"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowChunksEmptyStates(
	TEXT("voxel.renderer.ShowChunksEmptyStates"),
	0,
	TEXT("If true, will show updated chunks empty state, only if non-empty. Use ShowAllChunksEmptyStates to show empty too."),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowAllChunksEmptyStates(
	TEXT("voxel.renderer.ShowAllChunksEmptyStates"),
	0,
	TEXT("If true, will show updated chunks empty state, both empty and non-empty. Use ShowChunksEmptyStates to only show non-empty ones"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowWorldBounds(
	TEXT("voxel.ShowWorldBounds"),
	0,
	TEXT("If true, will show the world bounds"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowInvokers(
	TEXT("voxel.ShowInvokers"),
	0,
	TEXT("If true, will show the voxel invokers"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowDirtyVoxels(
	TEXT("voxel.data.ShowDirtyVoxels"),
	0,
	TEXT("If true, will show every dirty voxel in the scene"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowPlaceableItemsChunks(
	TEXT("voxel.data.ShowPlaceableItemsChunks"),
	0,
	TEXT("If true, will show every chunk that has a placeable item"),
	ECVF_Default);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
inline FConsoleCommandWithWorldAndArgsDelegate CreateCommandWithVoxelWorldDelegate(T Lambda)
{
	return FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([=](const TArray<FString>& Args, UWorld* World)
	{
		for (TActorIterator<AVoxelWorld> It(World); It; ++It)
		{
			if (It->IsCreated())
			{
				Lambda(**It, Args);
			}
		}
	});
}

template<typename T>
inline FConsoleCommandWithWorldAndArgsDelegate CreateCommandWithVoxelWorldDelegateNoArgs(T Lambda)
{
	return CreateCommandWithVoxelWorldDelegate([&](AVoxelWorld& World, const TArray<FString>& Args) { Lambda(World); });
}

static FAutoConsoleCommandWithWorldAndArgs ClearChunksEmptyStatesCmd(
	TEXT("voxel.renderer.ClearChunksEmptyStates"),
	TEXT("Clear the empty states debug"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World) { World.GetDebugManager().ClearChunksEmptyStates(); }));

static FAutoConsoleCommandWithWorldAndArgs UpdateAllCmd(
	TEXT("voxel.renderer.UpdateAll"),
	TEXT("Update all the chunks in all the voxel world in the scene"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World) { UVoxelBlueprintLibrary::UpdateBounds(&World, FVoxelIntBox::Infinite); }));

static FAutoConsoleCommandWithWorldAndArgs RecomputeMeshPositionsCmd(
	TEXT("voxel.renderer.RecomputeMeshPositions"),
	TEXT("Recompute the positions of all the meshes in all the voxel world in the scene"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World) { World.GetRenderer().RecomputeMeshPositions(); }));

static FAutoConsoleCommandWithWorldAndArgs ForceLODsUpdateCmd(
	TEXT("voxel.renderer.ForceLODUpdate"),
	TEXT("Update the LODs"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World) { World.GetLODManager().ForceLODsUpdate(); }));

static FAutoConsoleCommandWithWorldAndArgs CacheAllValuesCmd(
	TEXT("voxel.data.CacheAllValues"),
	TEXT("Cache all values"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World)
		{
			UVoxelDataTools::CacheValues(&World, FVoxelIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs CacheAllMaterialsCmd(
	TEXT("voxel.data.CacheAllMaterials"),
	TEXT("Cache all materials"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World)
		{
			UVoxelDataTools::CacheMaterials(&World, FVoxelIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs ClearAllCachedValuesCmd(
	TEXT("voxel.data.ClearAllCachedValues"),
	TEXT("Clear all cached values"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World)
		{
			UVoxelDataTools::ClearCachedValues(&World, FVoxelIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs ClearAllCachedMaterialsCmd(
	TEXT("voxel.data.ClearAllCachedMaterials"),
	TEXT("Clear all cached materials"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World)
		{
			UVoxelDataTools::ClearCachedMaterials(&World, FVoxelIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs CheckForSingleValuesCmd(
	TEXT("voxel.data.CheckForSingleValues"),
	TEXT("Check if values in a chunk are all the same, and if so only store one"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World)
		{
			UVoxelDataTools::CheckForSingleValues(&World, FVoxelIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs CheckForSingleMaterialsCmd(
	TEXT("voxel.data.CheckForSingleMaterials"),
	TEXT("Check if materials in a chunk are all the same, and if so only store one"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World)
		{
			UVoxelDataTools::CheckForSingleMaterials(&World, FVoxelIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs RoundVoxelsCmd(
	TEXT("voxel.data.RoundVoxels"),
	TEXT("Round all voxels that do not impact the surface nor the normals"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World)
		{
			UVoxelDataTools::RoundVoxels(&World, FVoxelIntBox::Infinite);
			if (World.GetData().bEnableUndoRedo) UVoxelBlueprintLibrary::SaveFrame(&World);
		}));

static FAutoConsoleCommandWithWorldAndArgs ClearUnusedMaterialsCmd(
	TEXT("voxel.data.ClearUnusedMaterials"),
	TEXT("Will clear all materials that do not affect the surface to improve compression"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World)
		{
			UVoxelDataTools::ClearUnusedMaterials(&World, FVoxelIntBox::Infinite);
			if (World.GetData().bEnableUndoRedo) UVoxelBlueprintLibrary::SaveFrame(&World);
		}));

static FAutoConsoleCommandWithWorldAndArgs RegenerateAllSpawnersCmd(
	TEXT("voxel.spawners.RegenerateAll"),
	TEXT("Regenerate all spawners that can be regenerated"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World)
		{
			UVoxelBlueprintLibrary::RegenerateSpawners(&World, FVoxelIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs CompressIntoHeightmapCmd(
	TEXT("voxel.data.CompressIntoHeightmap"),
	TEXT("Update the heightmap to match the voxel world data"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World)
		{
			UVoxelDataTools::CompressIntoHeightmap(&World);
			UVoxelBlueprintLibrary::UpdateBounds(&World, FVoxelIntBox::Infinite);
			if (World.GetData().bEnableUndoRedo) UVoxelBlueprintLibrary::SaveFrame(&World);
		}));

static FAutoConsoleCommandWithWorldAndArgs RoundToGeneratorCmd(
	TEXT("voxel.data.RoundToGenerator"),
	TEXT("Set the voxels back to the generator value if all the voxels in a radius of 2 have the same sign as the generator"),
	CreateCommandWithVoxelWorldDelegateNoArgs([](AVoxelWorld& World)
		{
			UVoxelDataTools::RoundToGenerator(&World, FVoxelIntBox::Infinite);
			UVoxelBlueprintLibrary::UpdateBounds(&World, FVoxelIntBox::Infinite);
			if (World.GetData().bEnableUndoRedo) UVoxelBlueprintLibrary::SaveFrame(&World);
		}));

static bool GShowCollisionAndNavmeshDebug = false;

static FAutoConsoleCommandWithWorldAndArgs ShowCollisionAndNavmeshDebugCmd(
	TEXT("voxel.renderer.ShowCollisionAndNavmeshDebug"),
	TEXT("If true, will show chunks used for collisions/navmesh and will color all chunks according to their usage"),
	CreateCommandWithVoxelWorldDelegate([](AVoxelWorld& World, const TArray<FString>& Args)
		{
			if (Args.Num() == 0)
			{
				GShowCollisionAndNavmeshDebug = !GShowCollisionAndNavmeshDebug;
			}
			else if (Args[0] == "0")
			{
				GShowCollisionAndNavmeshDebug = false;
			}
			else
			{
				GShowCollisionAndNavmeshDebug = true;
			}

			World.GetLODManager().UpdateBounds(FVoxelIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorld RebaseOntoCameraCmd(
	TEXT("voxel.RebaseOntoCamera"),
	TEXT("Call SetWorldOriginLocation so that the camera is at 0 0 0"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		auto* CameraManager = UGameplayStatics::GetPlayerCameraManager(World, 0);
		if (ensure(CameraManager))
		{
			const FVector Position = CameraManager->GetCameraLocation();
			UGameplayStatics::SetWorldOriginLocation(World, UGameplayStatics::GetWorldOriginLocation(World) + FIntVector(Position));
		}
	}));

static FAutoConsoleCommand CmdDestroyGlobalThreadPool(
    TEXT("voxel.threading.DestroyGlobalPool"),
    TEXT("Destroy the global thread pool"),
    FConsoleCommandDelegate::CreateStatic(&IVoxelPool::DestroyGlobalPool));

static FAutoConsoleCommandWithWorld CmdDestroyWorldThreadPool(
    TEXT("voxel.threading.DestroyWorldPool"),
    TEXT("Destroy the current world thread pool"),
	FConsoleCommandWithWorldDelegate::CreateStatic(&IVoxelPool::DestroyWorldPool));

static FAutoConsoleCommand CmdLogThreadPoolStats(
    TEXT("voxel.threading.LogStats"),
    TEXT(""),
	FConsoleCommandDelegate::CreateLambda([](){ FVoxelQueuedThreadPoolStats::Get().LogTimes(); }));

static FAutoConsoleCommand CmdLogMemoryStats(
    TEXT("voxel.LogMemoryStats"),
    TEXT(""),
    FConsoleCommandDelegate::CreateStatic(&UVoxelBlueprintLibrary::LogMemoryStats));

static void LogSecondsPerCycles()
{
    LOG_VOXEL(Log, TEXT("SECONDS PER CYCLES: %e"), FPlatformTime::GetSecondsPerCycle());
}

static FAutoConsoleCommand CmdLogSecondsPerCycles(
    TEXT("voxel.debug.LogSecondsPerCycles"),
    TEXT(""),
    FConsoleCommandDelegate::CreateStatic(&LogSecondsPerCycles));

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline float GetBoundsThickness(const FVoxelIntBox& Bounds)
{
	return Bounds.Size().GetMax();
}

#define DRAW_BOUNDS(Bounds, Color, bThick) UVoxelDebugUtilities::DrawDebugIntBox(World, Bounds, DebugDT, bThick ? GetBoundsThickness(Bounds) : 0, FLinearColor(Color));
#define DRAW_BOUNDS_ARRAY(BoundsArray, Color, bThick) for (auto& Bounds : BoundsArray) { DRAW_BOUNDS(Bounds, FColorList::Color, bThick) }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelDebugManagerSettings::FVoxelDebugManagerSettings(
	const AVoxelWorld* World,
	EVoxelPlayType PlayType,
	const TVoxelSharedRef<IVoxelPool>& Pool,
	const TVoxelSharedRef<FVoxelData>& Data,
	bool bDisabled)
	: VoxelWorld(const_cast<AVoxelWorld*>(World))
	, Pool(Pool)
	, Data(Data)
	, bDisabled(bDisabled)
{
}

TVoxelSharedRef<FVoxelDebugManager> FVoxelDebugManager::Create(const FVoxelDebugManagerSettings& Settings)
{
	return MakeShareable(new FVoxelDebugManager(Settings));
}

void FVoxelDebugManager::Destroy()
{
	StopTicking();
}

FVoxelDebugManager::FVoxelDebugManager(const FVoxelDebugManagerSettings& Settings)
	: Settings(Settings)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDebugManager::ReportUpdatedChunks(TFunction<TArray<FVoxelIntBox>()> InUpdatedChunks)
{
	if (CVarShowUpdatedChunks.GetValueOnGameThread())
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();
		UpdatedChunks = InUpdatedChunks();
	
		FString Log = "Updated chunks: ";
		for (auto& Bounds : UpdatedChunks)
		{
			Log += Bounds.ToString() + "; ";
		}
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), 1, FColor::Blue, Log);
		LOG_VOXEL(Log, TEXT("%s"), *Log);
	}
}

void FVoxelDebugManager::ReportRenderChunks(TFunction<TArray<FVoxelIntBox>()> InRenderChunks)
{
	if (CVarShowRenderChunks.GetValueOnGameThread())
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();
		RenderChunks = InRenderChunks();
	}
}

void FVoxelDebugManager::ReportMultiplayerSyncedChunks(TFunction<TArray<FVoxelIntBox>()> InMultiplayerSyncedChunks)
{
	if (CVarShowMultiplayerSyncedChunks.GetValueOnGameThread())
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();
		MultiplayerSyncedChunks = InMultiplayerSyncedChunks();
	}
}

void FVoxelDebugManager::ReportMeshTaskCount(int32 InTaskCount)
{
	MeshTaskCount = InTaskCount;
}

void FVoxelDebugManager::ReportMeshTasksCallbacksQueueNum(int32 Num)
{
	MeshTasksCallbacksQueueNum = Num;
}

void FVoxelDebugManager::ReportMeshActionQueueNum(int32 Num)
{
	MeshActionQueueNum = Num;
}

void FVoxelDebugManager::ReportFoliageTaskCount(int32 TaskCount)
{
	FoliageTaskCount.Set(TaskCount);
}
				
void FVoxelDebugManager::ReportChunkEmptyState(const FVoxelIntBox& Bounds, bool bIsEmpty)
{
	ChunksEmptyStates.Emplace(FChunkEmptyState{ Bounds, bIsEmpty });
}

void FVoxelDebugManager::ClearChunksEmptyStates()
{
	ChunksEmptyStates.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelDebugManager::ShowCollisionAndNavmeshDebug()
{
	return GShowCollisionAndNavmeshDebug;
}

FColor FVoxelDebugManager::GetCollisionAndNavmeshDebugColor(bool bEnableCollisions, bool bEnableNavmesh)
{
	if (bEnableCollisions && bEnableNavmesh)
	{
		return FColor::Yellow;
	}
	if (bEnableCollisions)
	{
		return FColor::Blue;
	}
	if (bEnableNavmesh)
	{
		return FColor::Green;
	}
	return FColor::White;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDebugManager::Tick(float DeltaTime)
{
	VOXEL_FUNCTION_COUNTER();

	if (Settings.bDisabled) return;

	auto* World = Settings.VoxelWorld.Get();
	if (!World) return;
	if (World->bDisableDebugManager) return;

	const float DebugDT = DeltaTime * 1.5f * CVarDebugDrawTime.GetValueOnGameThread();

	if (CVarShowRenderChunks.GetValueOnGameThread())
	{
		DRAW_BOUNDS_ARRAY(RenderChunks, Grey, false);
	}
	if (CVarShowUpdatedChunks.GetValueOnGameThread())
	{
		DRAW_BOUNDS_ARRAY(UpdatedChunks, Blue, true);
	}
	if (CVarShowMultiplayerSyncedChunks.GetValueOnGameThread())
	{
		DRAW_BOUNDS_ARRAY(MultiplayerSyncedChunks, Blue, true);
	}
	if (CVarShowWorldBounds.GetValueOnGameThread())
	{
		DRAW_BOUNDS(Settings.Data->WorldBounds, FColorList::Red, true);
	}
	if (!World->bDisableOnScreenMessages)
	{
		const int32 PoolTaskCount = Settings.Pool->GetNumTasks();
		if (PoolTaskCount > 0)
		{
			GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, FColor::White, FString::Printf(TEXT("Total tasks remaining: %d"), PoolTaskCount));
		}
		if (MeshTaskCount > 0)
		{
			GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, FColor::White, FString::Printf(TEXT("Mesh tasks remaining: %d"), MeshTaskCount));
		}
		if (MeshTasksCallbacksQueueNum > 0)
		{
			GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, FColor::White, FString::Printf(TEXT("Mesh tasks callbacks queued: %d"), MeshTasksCallbacksQueueNum));
		}
		if (MeshActionQueueNum > 0)
		{
			GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, FColor::White, FString::Printf(TEXT("Mesh actions queued: %d"), MeshActionQueueNum));
		}
		if (FoliageTaskCount.GetValue() > 0)
		{
			GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, FColor::White, FString::Printf(TEXT("Foliage tasks remaining: %d"), FoliageTaskCount.GetValue()));
		}
	}
	if (!CVarFreezeDebug.GetValueOnGameThread())
	{
		UpdatedChunks.Reset();
		MultiplayerSyncedChunks.Reset();
	}

	if (CVarShowInvokers.GetValueOnGameThread())
	{
		const FColor LocalInvokerColor = FColor::Green;
		const FColor RemoteInvokerColor = FColor::Silver;
		const FColor LODColor = FColor::Red;
		const FColor CollisionsColor = FColor::Blue;
		const FColor NavmeshColor = FColor::Green;
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, LocalInvokerColor, TEXT("Local Invokers"));
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, RemoteInvokerColor, TEXT("Remote Invokers"));
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, LODColor, TEXT("Invokers LOD Bounds"));
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, CollisionsColor, TEXT("Invokers Collisions Bounds"));
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, NavmeshColor, TEXT("Invokers Navmesh Bounds"));
		
		for (auto& Invoker : UVoxelInvokerComponentBase::GetInvokers(World->GetWorld()))
		{
			if (Invoker.IsValid())
			{
				DrawDebugPoint(
					World->GetWorld(),
					World->LocalToGlobal(Invoker->GetInvokerVoxelPosition(World)),
					100,
					Invoker->IsLocalInvoker() ? LocalInvokerColor : RemoteInvokerColor,
					false,
					DebugDT);

				const auto InvokerSettings = Invoker->GetInvokerSettings(World);

				if (InvokerSettings.bUseForLOD)
				{
					DRAW_BOUNDS(InvokerSettings.LODBounds, LODColor, true);
				}
				if (InvokerSettings.bUseForCollisions)
				{
					DRAW_BOUNDS(InvokerSettings.CollisionsBounds, CollisionsColor, true);
				}
				if (InvokerSettings.bUseForNavmesh)
				{
					DRAW_BOUNDS(InvokerSettings.NavmeshBounds, NavmeshColor, true);
				}
			}
		}
	}

	if (CVarShowChunksEmptyStates.GetValueOnGameThread())
	{
		const static FColor NotEmpty = FColorList::Brown;

		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, NotEmpty, TEXT("Not empty chunks (range analysis failed)"));

		for (auto& EmptyState : ChunksEmptyStates)
		{
			if (!EmptyState.bIsEmpty)
			{
				DRAW_BOUNDS(EmptyState.Bounds, NotEmpty, false);
			}
		}
	}
	if (CVarShowAllChunksEmptyStates.GetValueOnGameThread())
	{
		const static FColor Empty = FColorList::Green;
		const static FColor NotEmpty = FColorList::Brown;

		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, Empty, TEXT("Empty chunks (range analysis successful)"));
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, NotEmpty, TEXT("Not empty chunks (range analysis failed)"));

		for (auto& EmptyState : ChunksEmptyStates)
		{
			if (EmptyState.bIsEmpty)
			{
				DRAW_BOUNDS(EmptyState.Bounds, Empty, false);
			}
			else
			{
				DRAW_BOUNDS(EmptyState.Bounds, NotEmpty, false);
			}
		}
	}

	const FColor SingleColor = FColorList::Green;
	const FColor SingleDirtyColor = FColorList::Blue;
	const FColor CachedColor = FColorList::Yellow;
	const FColor DirtyColor = FColorList::Red;
	
	const UVoxelDebugUtilities::FDrawDataOctreeSettings DrawDataOctreeSettings
	{
		World,
		DebugDT,
		false,
		false,
		SingleColor,
		SingleDirtyColor,
		CachedColor,
		DirtyColor
	};
	
	if (CVarShowValuesState.GetValueOnGameThread())
	{
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, FColor::White, "Values state:");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, DirtyColor, "Dirty");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, CachedColor, "Cached");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, SingleColor, "Single Item Stored");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, SingleDirtyColor, "Single Item Stored - Dirty");

		auto LocalDrawDataOctreeSettings = DrawDataOctreeSettings;
		LocalDrawDataOctreeSettings.bShowSingle = true;
		LocalDrawDataOctreeSettings.bShowCached = true;
		
		FVoxelReadScopeLock Lock(*Settings.Data, FVoxelIntBox::Infinite, FUNCTION_FNAME);
		UVoxelDebugUtilities::DrawDataOctreeImpl<FVoxelValue>(*Settings.Data, LocalDrawDataOctreeSettings);
	}
	if (CVarShowMaterialsState.GetValueOnGameThread())
	{
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, FColor::White, "Materials state:");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, DirtyColor, "Dirty");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, CachedColor, "Cached");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, SingleColor, "Single Item Stored");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, SingleDirtyColor, "Single Item Stored - Dirty");

		auto LocalDrawDataOctreeSettings = DrawDataOctreeSettings;
		LocalDrawDataOctreeSettings.bShowSingle = true;
		LocalDrawDataOctreeSettings.bShowCached = true;
		
		FVoxelReadScopeLock Lock(*Settings.Data, FVoxelIntBox::Infinite, FUNCTION_FNAME);
		UVoxelDebugUtilities::DrawDataOctreeImpl<FVoxelMaterial>(*Settings.Data, LocalDrawDataOctreeSettings);
	}
	
	if (CVarShowDirtyValues.GetValueOnGameThread())
	{
		auto LocalDrawDataOctreeSettings = DrawDataOctreeSettings;
		LocalDrawDataOctreeSettings.bShowSingle = false;
		LocalDrawDataOctreeSettings.bShowCached = false;
		
		FVoxelReadScopeLock Lock(*Settings.Data, FVoxelIntBox::Infinite, FUNCTION_FNAME);
		UVoxelDebugUtilities::DrawDataOctreeImpl<FVoxelValue>(*Settings.Data, LocalDrawDataOctreeSettings);
	}
	if (CVarShowDirtyMaterials.GetValueOnGameThread())
	{
		auto LocalDrawDataOctreeSettings = DrawDataOctreeSettings;
		LocalDrawDataOctreeSettings.bShowSingle = false;
		LocalDrawDataOctreeSettings.bShowCached = false;
		
		FVoxelReadScopeLock Lock(*Settings.Data, FVoxelIntBox::Infinite, FUNCTION_FNAME);
		UVoxelDebugUtilities::DrawDataOctreeImpl<FVoxelMaterial>(*Settings.Data, LocalDrawDataOctreeSettings);
	}

	if (CVarShowPlaceableItemsChunks.GetValueOnGameThread())
	{
		FVoxelReadScopeLock Lock(*Settings.Data, FVoxelIntBox::Infinite, FUNCTION_FNAME);
		FVoxelOctreeUtilities::IterateEntireTree(Settings.Data->GetOctree(), [&](const FVoxelDataOctreeBase& Octree)
		{
			if (Octree.IsLeafOrHasNoChildren() && Octree.GetItemHolder().NumItems() > 0)
			{
				ensureThreadSafe(Octree.IsLockedForRead());
				UVoxelDebugUtilities::DrawDebugIntBox(World, Octree.GetBounds(), DebugDT, 0, FColorList::Red);
			}
		});
	}
	
	if (GShowCollisionAndNavmeshDebug)
	{
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, GetCollisionAndNavmeshDebugColor(true, false), "Chunks with collisions");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, GetCollisionAndNavmeshDebugColor(false, true), "Chunks with navmesh");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, GetCollisionAndNavmeshDebugColor(true, true), "Chunks with navmesh and collision");
	}
	if (CVarShowDirtyVoxels.GetValueOnGameThread())
	{
		FVoxelDataUtilities::IterateDirtyDataInBounds<FVoxelValue>(
			*Settings.Data,
			FVoxelIntBox::Infinite,
			[&](int32 X, int32 Y, int32 Z, const FVoxelValue& Value)
			{
				DrawDebugPoint(
					World->GetWorld(),
					World->LocalToGlobal(FIntVector(X, Y, Z)),
					2,
					Value.IsEmpty() ? FColor::Blue : FColor::Red,
					false,
					DebugDT);
			});
	}
}