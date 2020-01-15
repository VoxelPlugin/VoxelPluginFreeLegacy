// Copyright 2020 Phyronnaz

#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelData/VoxelData.h"
#include "VoxelDebugUtilities.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelTools/VoxelDataTools.h"
#include "VoxelMessages.h"
#include "VoxelWorld.h"
#include "IVoxelPool.h"

#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"

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

static TAutoConsoleVariable<int32> CVarShowFoliageState(
	TEXT("voxel.data.ShowFoliageState"),
	0,
	TEXT("If true, will show the foliage data chunks and their status (cached/created...)"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarFreezeDebug(
	TEXT("voxel.FreezeDebug"),
	0,
	TEXT("If true, won't clear previous frames boxes"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowChunksEmptyStates(
	TEXT("voxel.renderer.ShowChunksEmptyStates"),
	0,
	TEXT("If true, will show updated chunks empty state"),
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

static TAutoConsoleVariable<int32> CVarShowCollisionAndNavmeshDebug(
	TEXT("voxel.renderer.ShowCollisionAndNavmeshDebug"),
	0,
	TEXT("If true, will show chunks used for collisions/navmesh and will color all chunks according to their usage"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowDirtyVoxels(
	TEXT("voxel.data.ShowDirtyVoxels"),
	0,
	TEXT("If true, will show every dirty voxel in the scene"),
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
				Lambda(**It);
			}
		}
	});
}

static FAutoConsoleCommandWithWorldAndArgs ClearChunksEmptyStatesCmd(
	TEXT("voxel.renderer.ClearChunksEmptyStates"),
	TEXT("Clear the empty states debug"),
	CreateCommandWithVoxelWorldDelegate([](AVoxelWorld& World) { World.GetDebugManager().ClearChunksEmptyStates(); }));

static FAutoConsoleCommandWithWorldAndArgs UpdateAllCmd(
	TEXT("voxel.renderer.UpdateAll"),
	TEXT("Update all the chunks in all the voxel world in the scene"),
	CreateCommandWithVoxelWorldDelegate([](AVoxelWorld& World) { World.GetLODManager().UpdateBounds(FIntBox::Infinite); }));

static FAutoConsoleCommandWithWorldAndArgs RecomputeMeshPositionsCmd(
	TEXT("voxel.renderer.RecomputeMeshPositions"),
	TEXT("Recompute the positions of all the meshes in all the voxel world in the scene"),
	CreateCommandWithVoxelWorldDelegate([](AVoxelWorld& World) { World.GetRenderer().RecomputeMeshPositions(); }));

static FAutoConsoleCommandWithWorldAndArgs ForceLODsUpdateCmd(
	TEXT("voxel.renderer.ForceLODUpdate"),
	TEXT("Update the LODs"),
	CreateCommandWithVoxelWorldDelegate([](AVoxelWorld& World) { World.GetLODManager().ForceLODsUpdate(); }));

static FAutoConsoleCommandWithWorldAndArgs CacheAllValuesCmd(
	TEXT("voxel.data.CacheAllValues"),
	TEXT("Cache all values"),
	CreateCommandWithVoxelWorldDelegate([](AVoxelWorld& World)
		{
			FVoxelWriteScopeLock Lock(World.GetData(), FIntBox::Infinite, "");
			World.GetData().CacheBounds<FVoxelValue>(FIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs CacheAllMaterialsCmd(
	TEXT("voxel.data.CacheAllMaterials"),
	TEXT("Cache all materials"),
	CreateCommandWithVoxelWorldDelegate([](AVoxelWorld& World)
		{
			FVoxelWriteScopeLock Lock(World.GetData(), FIntBox::Infinite, "");
			World.GetData().CacheBounds<FVoxelMaterial>(FIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs ClearAllCachedValuesCmd(
	TEXT("voxel.data.ClearAllCachedValues"),
	TEXT("Clear all cached values"),
	CreateCommandWithVoxelWorldDelegate([](AVoxelWorld& World)
		{
			FVoxelWriteScopeLock Lock(World.GetData(), FIntBox::Infinite, "");
			World.GetData().ClearCacheInBounds<FVoxelValue>(FIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs ClearAllCachedMaterialsCmd(
	TEXT("voxel.data.ClearAllCachedMaterials"),
	TEXT("Clear all cached materials"),
	CreateCommandWithVoxelWorldDelegate([](AVoxelWorld& World)
		{
			FVoxelWriteScopeLock Lock(World.GetData(), FIntBox::Infinite, "");
			World.GetData().ClearCacheInBounds<FVoxelMaterial>(FIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs CheckForSingleValuesCmd(
	TEXT("voxel.data.CheckForSingleValues"),
	TEXT("Check if values in a chunk are all the same, and if so only store one"),
	CreateCommandWithVoxelWorldDelegate([](AVoxelWorld& World)
		{
			FVoxelWriteScopeLock Lock(World.GetData(), FIntBox::Infinite, "");
			World.GetData().CheckIsSingle<FVoxelValue>(FIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs RoundVoxelsCmd(
	TEXT("voxel.data.RoundVoxels"),
	TEXT("Round all voxels that do not impact the surface nor the normals"),
	CreateCommandWithVoxelWorldDelegate([](AVoxelWorld& World)
		{
			UVoxelDataTools::RoundVoxels(&World, FIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs ClearUnusedMaterialsCmd(
	TEXT("voxel.data.ClearUnusedMaterials"),
	TEXT("Will clear all materials that do not affect the surface to improve compression"),
	CreateCommandWithVoxelWorldDelegate([](AVoxelWorld& World)
		{
			UVoxelDataTools::ClearUnusedMaterials(&World, FIntBox::Infinite);
		}));

static FAutoConsoleCommandWithWorldAndArgs CheckForSingleMaterialsCmd(
	TEXT("voxel.data.CheckForSingleMaterials"),
	TEXT("Check if materials in a chunk are all the same, and if so only store one"),
	CreateCommandWithVoxelWorldDelegate([](AVoxelWorld& World)
		{
			FVoxelWriteScopeLock Lock(World.GetData(), FIntBox::Infinite, "");
			World.GetData().CheckIsSingle<FVoxelMaterial>(FIntBox::Infinite);
		}));

static void LogSecondsPerCycles()
{
    UE_LOG(LogVoxel, Log, TEXT("SECONDS PER CYCLES: %e"), FPlatformTime::GetSecondsPerCycle());
}

static FAutoConsoleCommand CmdLogSecondsPerCycles(
    TEXT("voxel.debug.LogSecondsPerCycles"),
    TEXT(""),
    FConsoleCommandDelegate::CreateStatic(&LogSecondsPerCycles));

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline float GetBoundsThickness(const FIntBox& Bounds)
{
	return Bounds.Size().GetMax();
}

#define DRAW_BOUNDS(Bounds, Color, bThick) UVoxelDebugUtilities::DrawDebugIntBox(World, Bounds, DebugDT, bThick ? GetBoundsThickness(Bounds) : 0, FLinearColor(Color));
#define DRAW_BOUNDS_ARRAY(BoundsArray, Color, bThick) for (auto& Bounds : BoundsArray) { DRAW_BOUNDS(Bounds, FColorList::Color, bThick) }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static const FColor SingleColor = FColorList::Green;
static const FColor SingleDirtyColor = FColorList::Blue;
static const FColor CachedColor = FColorList::Yellow;
static const FColor DirtyColor = FColorList::Red;

template<typename T>
inline void DrawDataOctree(FVoxelDataOctreeBase& Octree, AVoxelWorld* World, float DebugDT)
{
	if (Octree.IsLeaf())
	{
		auto& Data = Octree.AsLeaf().GetData<T>();
		const auto Draw = [&](FColor Color)
		{
			DRAW_BOUNDS(Octree.GetBounds(), Color, false);
		};
		if (Data.IsSingleValue())
		{
			if (Data.IsDirty())
			{
				Draw(SingleDirtyColor);
			}
			else
			{
				Draw(SingleColor);
			}
		}
		else if (Data.GetDataPtr())
		{
			if (Data.IsDirty())
			{
				Draw(DirtyColor);
			}
			else
			{
				Draw(CachedColor);
			}
		}
	}
	else
	{
		auto& Parent = Octree.AsParent();
		if (Parent.HasChildren())
		{
			for (auto& Child : Parent.GetChildren())
			{
				DrawDataOctree<T>(Child, World, DebugDT);
			}
		}
	}
}

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

void FVoxelDebugManager::ReportUpdatedChunks(TFunction<TArray<FIntBox>()> InUpdatedChunks)
{
	if (CVarShowUpdatedChunks.GetValueOnGameThread())
	{
		VOXEL_FUNCTION_COUNTER();
		UpdatedChunks = InUpdatedChunks();
	
		FString Log = "Updated chunks: ";
		for (auto& Bounds : UpdatedChunks)
		{
			Log += Bounds.ToString() + "; ";
		}
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), 1, FColor::Blue, Log);
		UE_LOG(LogVoxel, Log, TEXT("%s"), *Log);
	}
}

void FVoxelDebugManager::ReportRenderChunks(TFunction<TArray<FIntBox>()> InRenderChunks)
{
	if (CVarShowRenderChunks.GetValueOnGameThread())
	{
		VOXEL_FUNCTION_COUNTER();
		RenderChunks = InRenderChunks();
	}
}

void FVoxelDebugManager::ReportMultiplayerSyncedChunks(TFunction<TArray<FIntBox>()> InMultiplayerSyncedChunks)
{
	if (CVarShowMultiplayerSyncedChunks.GetValueOnGameThread())
	{
		VOXEL_FUNCTION_COUNTER();
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
				
void FVoxelDebugManager::ReportChunkEmptyState(const FIntBox& Bounds, bool bIsEmpty)
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
	return CVarShowCollisionAndNavmeshDebug.GetValueOnAnyThread() != 0;
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

	const float DebugDT = DeltaTime * 1.5f;

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
		for (auto& Invoker : UVoxelInvokerComponent::GetInvokers(World->GetWorld()))
		{
			if (Invoker.IsValid())
			{
				FVector Position = World->LocalToGlobal(World->GlobalToLocal(Invoker->GetPosition()));
				DrawDebugPoint(
					World->GetWorld(),
					Position,
					100,
					Invoker->IsLocalInvoker() ? FColor::Green : FColor::Silver,
					false,
					DebugDT);
				UE_LOG(LogVoxel, Log, TEXT("Invoker %s (owner: %s): %s"), *Invoker->GetName(), Invoker->GetOwner() ? *Invoker->GetOwner()->GetName() : TEXT("invalid"), *Position.ToString());
			}
		}
	}

	if (CVarShowChunksEmptyStates.GetValueOnGameThread())
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
	if (CVarShowValuesState.GetValueOnGameThread())
	{
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, FColor::White, "Values state:");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, DirtyColor, "Dirty");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, CachedColor, "Cached");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, SingleColor, "Single Item Stored");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, SingleDirtyColor, "Single Item Stored - Dirty");

		FVoxelReadScopeLock Lock(*Settings.Data, FIntBox::Infinite, FUNCTION_FNAME);
		DrawDataOctree<FVoxelValue>(Settings.Data->GetOctree(), World, DebugDT);
	}
	if (CVarShowMaterialsState.GetValueOnGameThread())
	{
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, FColor::White, "Materials state:");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, DirtyColor, "Dirty");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, CachedColor, "Cached");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, SingleColor, "Single Item Stored");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, SingleDirtyColor, "Single Item Stored - Dirty");

		FVoxelReadScopeLock Lock(*Settings.Data, FIntBox::Infinite, FUNCTION_FNAME);
		DrawDataOctree<FVoxelMaterial>(Settings.Data->GetOctree(), World, DebugDT);
	}
	if (CVarShowFoliageState.GetValueOnGameThread())
	{
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, FColor::White, "Foliage state:");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, DirtyColor, "Dirty");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, CachedColor, "Cached");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, SingleColor, "Single Item Stored");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, SingleDirtyColor, "Single Item Stored - Dirty");

		FVoxelReadScopeLock Lock(*Settings.Data, FIntBox::Infinite, FUNCTION_FNAME);
		DrawDataOctree<FVoxelFoliage>(Settings.Data->GetOctree(), World, DebugDT);
	}
	if (CVarShowCollisionAndNavmeshDebug.GetValueOnGameThread())
	{
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, GetCollisionAndNavmeshDebugColor(true, false), "Chunks with collisions");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, GetCollisionAndNavmeshDebugColor(false, true), "Chunks with navmesh");
		GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), DebugDT, GetCollisionAndNavmeshDebugColor(true, true), "Chunks with navmesh and collision");
	}
	if (CVarShowDirtyVoxels.GetValueOnGameThread())
	{
		FVoxelDataUtilities::IterateDirtyDataInBounds<FVoxelValue>(
			*Settings.Data,
			FIntBox::Infinite,
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