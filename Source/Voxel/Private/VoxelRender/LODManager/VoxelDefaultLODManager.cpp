// Copyright 2019 Phyronnaz

#include "VoxelDefaultLODManager.h"
#include "VoxelRenderOctree.h"
#include "IVoxelPool.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelWorldInterface.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "Logging/MessageLog.h"

#define LOCTEXT_NAMESPACE "Voxel"

DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultLODManager::UpdateLODs"), STAT_FVoxelDefaultLODManager_UpdateLODs, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultLODManager::UpdateInvokers"), STAT_FVoxelDefaultLODManager_UpdateInvokers, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultLODManager::OnTaskFinished"), STAT_FVoxelDefaultLODManager_OnTaskFinished, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultLODManager::UpdateBounds"), STAT_FVoxelDefaultLODManager_UpdateBounds, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultLODManager::~FVoxelDefaultLODManager"), STAT_FVoxelDefaultLODManager_FVoxelDefaultLODManager, STATGROUP_Voxel);

TSharedRef<FVoxelDefaultLODManager> FVoxelDefaultLODManager::Create(
		const FVoxelLODSettings& LODSettings,
		TWeakObjectPtr<const AVoxelWorldInterface> VoxelWorldInterface,
		const TSharedPtr<FVoxelLODDynamicSettings>& DynamicSettings)
{
	TSharedRef<FVoxelDefaultLODManager> LODManager = MakeShareable(new FVoxelDefaultLODManager(LODSettings, VoxelWorldInterface, DynamicSettings));
	LODManager->StartTimer();
	return LODManager;
}

FVoxelDefaultLODManager::FVoxelDefaultLODManager(
	const FVoxelLODSettings& LODSettings,
	TWeakObjectPtr<const AVoxelWorldInterface> VoxelWorldInterface,
	const TSharedPtr<FVoxelLODDynamicSettings>& DynamicSettings)
	: IVoxelLODManager(LODSettings)
	, VoxelWorldInterface(VoxelWorldInterface)
	, DynamicSettings(DynamicSettings)
	, Task(MakeUnique<FVoxelRenderOctreeAsyncBuilder>(LODSettings.OctreeDepth, LODSettings.WorldBounds))
{
}

FVoxelDefaultLODManager::~FVoxelDefaultLODManager()
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelDefaultLODManager_FVoxelDefaultLODManager);
	if (!Task->IsDone())
	{
		Task->Autodelete();
		Task.Release();
	}
	if (Settings.World.IsValid())
	{
		auto& TimerManager = Settings.World->GetTimerManager();
		TimerManager.ClearTimer(UpdateHandle);
	}
}

///////////////////////////////////////////////////////////////////////////////

inline FIntBox GetBoundsToUpdate(const FIntBox& Bounds)
{
	return FIntBox(Bounds.Min - FIntVector(2, 2, 2), Bounds.Max + FIntVector(2, 2, 2));
}

void FVoxelDefaultLODManager::UpdateBounds(const FIntBox& Bounds, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelDefaultLODManager_UpdateBounds);

	if (!Octree.IsValid())
	{
		return;
	}
	TArray<uint64> ChunksToUpdate;
	Octree->GetChunksToUpdateForBounds(GetBoundsToUpdate(Bounds), ChunksToUpdate);
	Settings.Renderer->CancelDithering(Bounds, ChunksToUpdate);
	Settings.Renderer->UpdateChunks(ChunksToUpdate, Settings.bWaitForOtherChunksToAvoidHoles && bWaitForAllChunksToFinishUpdating, FinishDelegate);
}

void FVoxelDefaultLODManager::UpdateBounds(const TArray<FIntBox>& Bounds, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelDefaultLODManager_UpdateBounds);

	if (!Octree.IsValid() || Bounds.Num() == 0)
	{
		return;
	}

	TArray<uint64> ChunksToUpdate;
	FIntBox GlobalBounds = Bounds[0];
	for (auto& BoundsToUpdate : Bounds)
	{
		GlobalBounds += BoundsToUpdate;
		Octree->GetChunksToUpdateForBounds(GetBoundsToUpdate(BoundsToUpdate), ChunksToUpdate);
	}
	Settings.Renderer->CancelDithering(GlobalBounds, ChunksToUpdate);
	Settings.Renderer->UpdateChunks(ChunksToUpdate, Settings.bWaitForOtherChunksToAvoidHoles && bWaitForAllChunksToFinishUpdating, FinishDelegate);
}

void FVoxelDefaultLODManager::UpdatePosition(const FIntVector& Position, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate)
{
	UpdateBounds(FIntBox(Position), bWaitForAllChunksToFinishUpdating, FinishDelegate);
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelDefaultLODManager::ForceLODsUpdate()
{
	PreviousInvokers.Reset();
	PreviousInvokers.Add(nullptr);

	auto& TimerManager = Settings.World->GetTimerManager();
	if (TimerManager.IsTimerActive(UpdateHandle))
	{
		check(Task->IsDone());
		TimerManager.ClearTimer(UpdateHandle);
		UpdateLODs();
	}
}

bool FVoxelDefaultLODManager::AreCollisionsEnabled(const FIntVector& Position, uint8& OutLOD) const
{
	if (!Octree.IsValid())
	{
		return false;
	}
	auto* Ptr = Octree.Get();
	while (!Ptr->IsLeaf() && !Ptr->GetSettings().bEnableCollisions)
	{
		Ptr = &Ptr->GetChild(Position);
	}

	OutLOD = Ptr->LOD;
	return Ptr->GetSettings().bEnableCollisions;
}

void FVoxelDefaultLODManager::UpdateLODs()
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelDefaultLODManager_UpdateLODs);
	
	if (!VoxelWorldInterface.IsValid())
	{
		return;
	}

	if (UpdateInvokers())
	{
		check(Task->IsDone());
		if (!Task->Delegate.IsBound())
		{
			Task->Delegate.BindSP(this, &FVoxelDefaultLODManager::OnTaskFinished);
		}

		FVoxelRenderOctreeSettings OctreeSettings{};
		OctreeSettings.LODLimit = DynamicSettings->LODLimit;
		OctreeSettings.WorldBounds = Settings.WorldBounds;
		OctreeSettings.Invokers = Invokers;
		OctreeSettings.SquaredLODsDistances = SquaredLODsDistances;

		OctreeSettings.Invokers = Invokers;
		OctreeSettings.ChunksCullingLOD = DynamicSettings->ChunksCullingLOD;

		OctreeSettings.bEnableRender = DynamicSettings->bEnableRender;
		
		OctreeSettings.bEnableCollisions = DynamicSettings->bEnableCollisions;
		OctreeSettings.bComputeVisibleChunksCollisions = DynamicSettings->bComputeVisibleChunksCollisions;
		OctreeSettings.VisibleChunksCollisionsMaxLOD = DynamicSettings->VisibleChunksCollisionsMaxLOD;
		
		OctreeSettings.bEnableNavmesh = DynamicSettings->bEnableNavmesh;

		OctreeSettings.bEnableTessellation = DynamicSettings->bEnableTessellation;
		OctreeSettings.SquaredTessellationDistance = GetSquaredDistance(DynamicSettings->TessellationDistance);

		Task->Init(OctreeSettings, Octree);
		Settings.Pool->QueueOctreeBuildTask(Task.Get());
	}
	else
	{
		StartTimer();
	}
}

void FVoxelDefaultLODManager::OnTaskFinished()
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelDefaultLODManager_OnTaskFinished);

	Task->ReportBuildTime(*Settings.DebugManager);
	Octree = Task->NewOctree;
	Settings.Renderer->UpdateLODs(Task->ChunksToAdd, Task->ChunksToUpdate, Task->ChunksToRemove, Task->TransitionsToUpdate);
	StartTimer();
}

void FVoxelDefaultLODManager::StartTimer()
{
	if (Settings.World.IsValid())
	{
		auto& TimerManager = Settings.World->GetTimerManager();
		TimerManager.SetTimer(UpdateHandle, FTimerDelegate::CreateSP(this, &FVoxelDefaultLODManager::UpdateLODs), 1.f / Settings.LODUpdateRate, false);
	}
}

bool FVoxelDefaultLODManager::UpdateInvokers()
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelDefaultLODManager_UpdateInvokers);
	
	bool bNeedUpdate = false;
	
	TArray<TWeakObjectPtr<UVoxelInvokerComponent>> NewInvokers = UVoxelInvokerComponent::GetInvokers(Settings.World.Get());
	NewInvokers.Sort([](auto& A, auto& B) { return A.Get() < B.Get(); });

	if (!Octree.IsValid() || PreviousInvokers != NewInvokers)
	{
		bNeedUpdate = true;
	}
	else
	{
		for (auto& Invoker : NewInvokers)
		{
			auto& OldPosition = InvokersPreviousPositions[Invoker];
			auto NewPosition = Invoker->GetPosition();

			if (FVector::Dist(OldPosition, NewPosition) > DynamicSettings->InvokerDistanceThreshold)
			{
				bNeedUpdate = true;
				break;
			}
		}
	}

	if (bNeedUpdate)
	{
		PreviousInvokers = NewInvokers;

		InvokersPreviousPositions.Reset();
		
		auto& InvokersPositions = *Settings.Renderer->Settings.InvokersPositions;
		InvokersPositions.Reset();
		
		Invokers.Reset();

		for (auto& Invoker : NewInvokers)
		{
			auto GlobalPosition = Invoker->GetPosition();
			auto LocalPosition = VoxelWorldInterface->GlobalToLocal(GlobalPosition);

			InvokersPreviousPositions.Add(Invoker, GlobalPosition);
			InvokersPositions.Add(LocalPosition);

			FVoxelInvoker InvokerSettings;
			InvokerSettings.Position = LocalPosition;
			InvokerSettings.bUseForLODs = Invoker->bUseForLODs && Invoker->IsLocalInvoker();
			InvokerSettings.bUseForCollisions = Invoker->bUseForCollisions;
			InvokerSettings.SquaredCollisionsRange = GetSquaredDistance(Invoker->CollisionsRange);
			InvokerSettings.bUseForNavmesh = Invoker->bUseForNavmesh;
			InvokerSettings.SquaredNavmeshRange = GetSquaredDistance(Invoker->NavmeshRange);
			InvokerSettings.SquaredGenerationRange = GetSquaredDistance(Invoker->GenerationRange);
			Invokers.Emplace(InvokerSettings);
		}
		
		SquaredLODsDistances.SetNum(MAX_WORLD_DEPTH);
		auto& LODToMinDistance = DynamicSettings->LODToMinDistance;
		for (int32 Index = 1; Index < MAX_WORLD_DEPTH; Index++)
		{
			// -1: We want to divide LOD 2 if LOD _1_ min distance isn't met, not if LOD _2_ min distance isn't met
			int32 CurrentLOD = Index - 1;
			while (!LODToMinDistance.Contains(CurrentLOD))
			{
				check(CurrentLOD > 0);
				CurrentLOD--;
			}
			SquaredLODsDistances[Index] = GetSquaredDistance(LODToMinDistance.FindChecked(CurrentLOD));
		}
	}

	return bNeedUpdate;
}

uint64 FVoxelDefaultLODManager::GetSquaredDistance(float DistanceInCm) const
{
	return FMath::Square<uint64>(FMath::CeilToInt(DistanceInCm / Settings.VoxelSize));
}

#undef LOCTEXT_NAMESPACE