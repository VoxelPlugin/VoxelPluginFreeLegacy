// Copyright 2019 Phyronnaz

#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelData/VoxelData.h"
#include "VoxelDebugUtilities.h"
#include "VoxelRender/Renderers/VoxelRenderChunk.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelWorld.h"

#include "Engine/Engine.h"
#include "Logging/MessageLog.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"

#define LOCTEXT_NAMESPACE "Voxel"

DECLARE_CYCLE_STAT(TEXT("FVoxelDebugManager::Tick"), STAT_VoxelDebugManager_Tick, STATGROUP_Voxel);

///////////////////////////////////////////////////////////////////////////////

static TAutoConsoleVariable<int32> CVarShowUpdatedChunks(
	TEXT("voxel.ShowUpdatedChunks"),
	0,
	TEXT("If true, will show the chunks recently updated"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowRenderChunks(
	TEXT("voxel.ShowRenderChunks"),
	0,
	TEXT("If true, will show the render chunks"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowDataChunks(
	TEXT("voxel.ShowDataChunks"),
	0,
	TEXT("If true, will show the data chunks and their status (cached/created...)"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarLogCacheStats(
	TEXT("voxel.LogCacheStats"),
	0,
	TEXT("If true, will log all the voxel cache stats"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowTaskCount(
	TEXT("voxel.ShowTaskCount"),
	1,
	TEXT("If true, will log the remaining tasks count"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowMultiplayerSyncedBounds(
	TEXT("voxel.ShowMultiplayerSyncedBounds"),
	0,
	TEXT("If true, will show synced areas"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarFreezeDebug(
	TEXT("voxel.FreezeDebug"),
	0,
	TEXT("If true, won't clear previous frames boxes"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowChunksEmptyStates(
	TEXT("voxel.ShowChunksEmptyStates"),
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

static TAutoConsoleVariable<int32> CVarLogRenderOctreeBuildTime(
	TEXT("voxel.LogRenderOctreeBuildTime"),
	0,
	TEXT("If true, will log the render octree build times"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarLogToolsFailures(
	TEXT("voxel.LogToolsFailures"),
	1,
	TEXT("If true, will log each time a voxel tool fails"),
	ECVF_Default);


///////////////////////////////////////////////////////////////////////////////

static void	ClearChunksEmptyStates(const TArray<FString>& Args, UWorld* World)
{
	for (TActorIterator<AVoxelWorld> It(World); It; ++It)
	{
		It->GetDebugManager().ClearChunksEmptyStates();
	}
}

FAutoConsoleCommandWithWorldAndArgs ClearChunksEmptyStatesCmd(
	TEXT("voxel.ClearChunksEmptyStates"),
	TEXT("Clear the empty states debug"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(ClearChunksEmptyStates));

static void	UpdateAll(const TArray<FString>& Args, UWorld* World)
{
	for (TActorIterator<AVoxelWorld> It(World); It; ++It)
	{
		It->GetLODManager().UpdateBounds(FIntBox::Infinite);
	}
}

FAutoConsoleCommandWithWorldAndArgs UpdateAllCmd(
	TEXT("voxel.UpdateAll"),
	TEXT("Update all the chunks in all the voxel world in the scene"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(UpdateAll));

static void LogSecondsPerCycles()
{
	double SecondsPerCycles = FPlatformTime::GetSecondsPerCycle();
    UE_LOG(LogVoxel, Log, TEXT("SECONDS PER CYCLES: %e"), SecondsPerCycles);
}

FAutoConsoleCommand CmdLogSecondsPerCycles(
    TEXT("voxel.LogSecondsPerCycles"),
    TEXT(""),
    FConsoleCommandDelegate::CreateStatic(&LogSecondsPerCycles)
    );

///////////////////////////////////////////////////////////////////////////////

inline float GetBoundsThickness(const FIntBox& Bounds)
{
	return Bounds.Size().GetMax();
}

#define DRAW_BOUNDS(Bounds, Color, bThick) UVoxelDebugUtilities::DrawDebugIntBox(World, Bounds, DebugDT, bThick ? GetBoundsThickness(Bounds) : 0, FLinearColor(Color));

#define DRAW_BOUNDS_ARRAY(BoundsArray, Color, bThick) for (auto& Bounds : BoundsArray) { DRAW_BOUNDS(Bounds, FColorList::Color, bThick) }

#define DEBUG_MESSAGE_INDEX() ((uint64)this + __LINE__)

///////////////////////////////////////////////////////////////////////////////

const FColor CreatedDirtyValuesColor = FColor(255, 0, 0, 255);
const FColor CreatedDirtyMaterialsColor = FColor(255, 127, 0, 255);
const FColor CreatedDirtyValuesMaterialsColor = FColor(255, 255, 0, 255);

const FColor CreatedManualCachedValuesColor = FColor(127, 0, 255, 255);
const FColor CreatedManualCachedMaterialsColor = FColor(255, 0, 127, 255);
const FColor CreatedManualCachedValuesMaterialsColor = FColor(255, 0, 255, 255);

const FColor CreatedAutoCachedValuesColor = FColor(0, 127, 255, 255);
const FColor CreatedAutoCachedMaterialsColor = FColor(0, 255, 127, 255);
const FColor CreatedAutoCachedValuesMaterialsColor = FColor(0, 255, 255, 255);

bool GetDataOctreeColor(FColor& OutColor, AVoxelWorld* World, float DebugDT, FVoxelDataOctree& Octree, bool bForcePrint = false)
{
	if (Octree.IsLeaf())
	{
		if (Octree.LOD == 0)
		{
			if (Octree.IsCreated())
			{
				if (Octree.IsCacheOnly())
				{
					if (Octree.IsManuallyCached())
					{
						OutColor =
							Octree.AreValuesCreated() && Octree.AreMaterialsCreated()
							? CreatedManualCachedValuesMaterialsColor
							: Octree.AreValuesCreated()
							? CreatedManualCachedValuesColor
							: Octree.AreMaterialsCreated()
							? CreatedManualCachedMaterialsColor
							: FColor::White;
						return true;
					}
					else
					{
						OutColor =
							Octree.AreValuesCreated() && Octree.AreMaterialsCreated()
							? CreatedAutoCachedValuesMaterialsColor
							: Octree.AreValuesCreated()
							? CreatedAutoCachedValuesColor
							: Octree.AreMaterialsCreated()
							? CreatedAutoCachedMaterialsColor
							: FColor::White;
						return true;
					}
				}
				else
				{
					OutColor =
						Octree.AreValuesCreated() && Octree.AreMaterialsCreated()
						? CreatedDirtyValuesMaterialsColor
						: Octree.AreValuesCreated()
						? CreatedDirtyValuesColor
						: Octree.AreMaterialsCreated()
						? CreatedDirtyMaterialsColor
						: FColor::White;
					return true;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		FColor Color;
		bool bFirst = true;
		bool bMerge = true;
		TArray<FVoxelDataOctree*> ChildToPrint;
		TArray<FColor> ChildColors;
		for (auto& Child : Octree.GetChildren())
		{
			FColor ChildColor;
			if (GetDataOctreeColor(ChildColor, World, DebugDT, Child))
			{
				ChildToPrint.Add(&Child);
				ChildColors.Add(ChildColor);
			}
			else
			{
				bMerge = false;
			}
			if (bFirst)
			{
				bFirst = false;
				Color = ChildColor;
			}
			else if (Color != ChildColor)
			{
				bMerge = false;
			}
		}

		if (bMerge && !bForcePrint && false)
		{
			OutColor = Color;
			return true;
		}
		else
		{
			for (int32 Index = 0; Index < ChildToPrint.Num() ; Index++)
			{
				DRAW_BOUNDS(ChildToPrint[Index]->GetBounds(), ChildColors[Index], false);
			}
			return false;
		}
	}
}

void DebugDataOctreee(AVoxelWorld* World, float DebugDT, FVoxelDataOctree& Octree)
{
	FColor Dummy;
	GetDataOctreeColor(Dummy, World, DebugDT, Octree, true);
}


void FVoxelDebugManager::Tick(float DeltaTime, AVoxelWorld* World)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelDebugManager_Tick);

	const float DebugDT = DeltaTime * 1.5f;

	if (CVarShowRenderChunks.GetValueOnGameThread())
	{
		DRAW_BOUNDS_ARRAY(RenderChunks, Grey, false);
	}
	if (CVarShowUpdatedChunks.GetValueOnGameThread())
	{
		DRAW_BOUNDS_ARRAY(UpdatedChunks, Blue, true);
	}
	if (CVarShowMultiplayerSyncedBounds.GetValueOnGameThread())
	{
		DRAW_BOUNDS_ARRAY(MultiplayerSyncedChunks, DarkPurple, true);
	}
	if (CVarShowWorldBounds.GetValueOnGameThread())
	{
		DRAW_BOUNDS(World->GetData().WorldBounds, FColorList::Red, true);
	}
	if (CVarShowTaskCount.GetValueOnGameThread() && TaskCount > 0)
	{
		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), DebugDT, FColor::White, FString::Printf(TEXT("Tasks remaining: %d"), TaskCount));
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
					DebugDT);
				UE_LOG(LogVoxel, Log, TEXT("Invoker %s (owner: %s): %s"), *Invoker->GetName(), Invoker->GetOwner() ? *Invoker->GetOwner()->GetName() : TEXT("invalid"), *Position.ToString());
			}
		}
	}

	if (CVarShowChunksEmptyStates.GetValueOnGameThread())
	{
		const static FColor Empty = FColorList::Green;
		const static FColor NotEmpty = FColorList::Brown;

		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), DebugDT, Empty, TEXT("Empty chunks (range analysis successful)"));
		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), DebugDT, NotEmpty, TEXT("Not empty chunks (range analysis failed)"));

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
	if (CVarShowDataChunks.GetValueOnGameThread())
	{
		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), DebugDT, CreatedDirtyValuesColor, "Dirty values");
		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), DebugDT, CreatedDirtyMaterialsColor, "Dirty materials");
		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), DebugDT, CreatedDirtyValuesMaterialsColor, "Dirty values and materials");

		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), DebugDT, CreatedManualCachedValuesColor, "Manually cached values");
		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), DebugDT, CreatedManualCachedMaterialsColor, "Manually cached materials");
		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), DebugDT, CreatedManualCachedValuesMaterialsColor, "Manually cached values and materials");

		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), DebugDT, CreatedAutoCachedValuesColor, "Automatically cached values");
		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), DebugDT, CreatedAutoCachedMaterialsColor, "Automatically cached materials");
		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), DebugDT, CreatedAutoCachedValuesMaterialsColor, "Automatically cached values and materials");

		auto& Data = World->GetData();
		FVoxelReadScopeLock Lock(Data, FIntBox::Infinite, "DebugCache");
		DebugDataOctreee(World, DebugDT, *Data.GetOctree());
	}
}

void FVoxelDebugManager::ReportUpdatedChunks(const TArray<FIntBox>& InUpdatedChunks)
{
	UpdatedChunks = InUpdatedChunks;
	
	if (CVarShowUpdatedChunks.GetValueOnGameThread())
	{
		FString Log = "Updated chunks: ";
		for (auto& Bounds : UpdatedChunks)
		{
			Log += Bounds.ToString() + "; ";
		}
		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), 1, FColor::Blue, Log);
		UE_LOG(LogVoxel, Log, TEXT("%s"), *Log);
	}
}

void FVoxelDebugManager::ReportRenderChunks(const TMap<uint64, TSharedPtr<FVoxelRenderChunk, ESPMode::ThreadSafe>>& InRenderChunks)
{
	RenderChunks.Reset();
	for (auto& It : InRenderChunks)
	{
		auto& Bounds = It.Value->GetBounds();
		RenderChunks.Add(Bounds);
	}
}

void FVoxelDebugManager::ReportRenderOctreeBuild(const FString& Log, int32 NumberOfLeaves, bool bTooManyChunks)
{
	if (CVarLogRenderOctreeBuildTime.GetValueOnGameThread())
	{
		UE_LOG(LogVoxel, Log, TEXT("%s"), *Log);
	}

	if (bTooManyChunks)
	{
		FMessageLog("PIE").Error(FText::Format(LOCTEXT("RenderOctreeStopped",
			"Render octree update was stopped! Max render octree chunks count reached ({0}). This is likely caused by too demanding LOD settings. \n"
			"You can try the following: \n"
			"- reduce your world size\n"
			"- reduce your LODToMinDistance values\n"
			"- reduce your invokers collisions & navmesh distances\n"
			"- increase your LOD Limit\n"
			"- if you know what you're doing, you can increase the limit using voxel.MaxRenderOctreeChunks"),
			FText::FromString(FString::FromInt(NumberOfLeaves))));
	}
}

void FVoxelDebugManager::ReportOctreeCompact(float Duration, uint32 NumDeleted)
{
	FString String = FString::Printf(TEXT("Octree compacted. Time: %.3fms; Octrees deleted: %d"), Duration * 1000, NumDeleted);
	GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), 1, FColor::White, String);
	UE_LOG(LogVoxel, Log, TEXT("%s"), *String);
}

void FVoxelDebugManager::ReportCacheUpdate(
	uint32 NumChunksSubdivided,
	uint32 NumChunksCached,
	uint32 NumRemovedFromCache,
	uint32 TotalNumCachedChunks,
	uint32 MaxCacheSize,
	float Duration)
{
	if (CVarLogCacheStats.GetValueOnGameThread())
	{
		FString String = FString::Printf(
			TEXT("Cache stats: %d data chunks subdivided; %d cached; %d removed from cache; Cache usage: %d/%d (%d/%dMB, %.2f%%); Time: %.3fms"),
			NumChunksSubdivided,
			NumChunksCached,
			NumRemovedFromCache,
			TotalNumCachedChunks,
			MaxCacheSize,
			FVoxelDataCellUtilities::GetCacheSizeInMB(TotalNumCachedChunks),
			FVoxelDataCellUtilities::GetCacheSizeInMB(MaxCacheSize),
			100 * double(TotalNumCachedChunks) / MaxCacheSize,
			Duration * 1000.f);

		GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), 1, FColor::White, String);
		UE_LOG(LogVoxel, Log, TEXT("%s"), *String);
	}
}

void FVoxelDebugManager::ReportManualCacheProgress(int32 Current, int32 Total)
{
	GEngine->AddOnScreenDebugMessage(DEBUG_MESSAGE_INDEX(), 0.1, FColor::White, FString::Printf(TEXT("Building cache: %d/%d"), Current, Total));
}

void FVoxelDebugManager::ReportTasksCount(int32 InTaskCount)
{
	TaskCount = InTaskCount;
}

void FVoxelDebugManager::ReportMultiplayerSyncedChunks(const TArray<FIntBox>& InMultiplayerSyncedChunks)
{
	MultiplayerSyncedChunks = InMultiplayerSyncedChunks;
}
				
void FVoxelDebugManager::ReportChunkEmptyState(const FIntBox& Bounds, bool bIsEmpty)
{
	ChunksEmptyStates.Emplace(FChunkEmptyState{ Bounds, bIsEmpty });
}

void FVoxelDebugManager::ClearChunksEmptyStates()
{
	ChunksEmptyStates.Reset();
}

void FVoxelDebugManager::ReportToolFailure(const FString& ToolName, const FString& Message)
{
	if (CVarLogToolsFailures.GetValueOnGameThread())
	{
		UE_LOG(LogVoxel, Log, TEXT("%s"), *(ToolName + " failed: " + Message));
	}
}

#undef LOCTEXT_NAMESPACE