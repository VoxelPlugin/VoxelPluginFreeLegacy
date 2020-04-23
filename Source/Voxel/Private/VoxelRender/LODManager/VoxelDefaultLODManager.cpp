// Copyright 2020 Phyronnaz

#include "VoxelRender/LODManager/VoxelDefaultLODManager.h"
#include "VoxelRender/LODManager/VoxelRenderOctree.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "IntBox.h"
#include "IVoxelPool.h"
#include "VoxelWorldInterface.h"
#include "VoxelComponents/VoxelInvokerComponent.h"

DECLARE_DWORD_COUNTER_STAT(TEXT("Voxel Chunk Updates"), STAT_VoxelChunkUpdates, STATGROUP_VoxelCounters);

TVoxelSharedRef<FVoxelDefaultLODManager> FVoxelDefaultLODManager::Create(
	const FVoxelLODSettings& LODSettings,
	TWeakObjectPtr<const AVoxelWorldInterface> VoxelWorldInterface,
	const TVoxelSharedRef<FVoxelLODDynamicSettings>& DynamicSettings)
{
	TVoxelSharedRef<FVoxelDefaultLODManager> Result = MakeShareable(
		new FVoxelDefaultLODManager(
			LODSettings,
			VoxelWorldInterface,
			DynamicSettings));

	UVoxelInvokerComponentBase::OnForceRefreshInvokers.AddThreadSafeSP(Result, &FVoxelDefaultLODManager::ClearInvokerComponents);
	return Result;
}

FVoxelDefaultLODManager::FVoxelDefaultLODManager(
	const FVoxelLODSettings& LODSettings,
	TWeakObjectPtr<const AVoxelWorldInterface> VoxelWorldInterface,
	const TVoxelSharedRef<FVoxelLODDynamicSettings>& DynamicSettings)
	: IVoxelLODManager(LODSettings) 
	, VoxelWorldInterface(VoxelWorldInterface)
	, DynamicSettings(DynamicSettings)
	, Task(TUniquePtr<FVoxelRenderOctreeAsyncBuilder, TVoxelAsyncWorkDelete<FVoxelRenderOctreeAsyncBuilder>>(
		new FVoxelRenderOctreeAsyncBuilder(LODSettings.OctreeDepth, LODSettings.WorldBounds)))
{
}

FVoxelDefaultLODManager::~FVoxelDefaultLODManager()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread()); // FTickable
	if (!Task->IsDone())
	{
		Task->CancelAndAutodelete();
		Task.Release();
	}
}

///////////////////////////////////////////////////////////////////////////////

inline FIntBox GetBoundsToUpdate(const FIntBox& Bounds)
{
	// For normals
	return Bounds.Extend(2);
}

int32 FVoxelDefaultLODManager::UpdateBounds(const FIntBox& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!Octree.IsValid())
	{
		return 0;
	}

	TArray<uint64> ChunksToUpdate;
	Octree->GetChunksToUpdateForBounds(GetBoundsToUpdate(Bounds), ChunksToUpdate, OnChunkUpdate);
	return Settings.Renderer->UpdateChunks(Bounds, ChunksToUpdate, FinishDelegate);
}

int32 FVoxelDefaultLODManager::UpdateBounds(const TArray<FIntBox>& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!Octree.IsValid() || Bounds.Num() == 0)
	{
		return 0;
	}

	TArray<uint64> ChunksToUpdate;
	FIntBox GlobalBounds = Bounds[0];
	for (auto& BoundsToUpdate : Bounds)
	{
		GlobalBounds = GlobalBounds + BoundsToUpdate;
		Octree->GetChunksToUpdateForBounds(GetBoundsToUpdate(BoundsToUpdate), ChunksToUpdate, OnChunkUpdate);
	}
	return Settings.Renderer->UpdateChunks(GlobalBounds, ChunksToUpdate, FinishDelegate);
}

void FVoxelDefaultLODManager::ForceLODsUpdate()
{
	bLODUpdateQueued = true;
}

bool FVoxelDefaultLODManager::AreCollisionsEnabled(const FIntVector& Position, uint8& OutLOD) const
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!Octree.IsValid())
	{
		return false;
	}

	OutLOD = 255;

	auto* Ptr = Octree.Get();
	while (Ptr->HasChildren())
	{
		Ptr = &Ptr->GetChild(Position);
		if (Ptr->GetSettings().bEnableCollisions)
		{
			OutLOD = Ptr->Height;
		}
	}

	return OutLOD != 255;
}

void FVoxelDefaultLODManager::Destroy()
{
	if (IsTicking())
	{
		StopTicking();
	}
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelDefaultLODManager::Tick(float DeltaTime)
{
	VOXEL_FUNCTION_COUNTER();
	
	const double Time = FPlatformTime::Seconds();
	if (Time - LastInvokersUpdateTime > Settings.MinDelayBetweenLODUpdates)
	{
		LastInvokersUpdateTime = Time;
		UpdateInvokers();
	}

	if (bAsyncTaskWorking && Task->IsDone())
	{
		VOXEL_SCOPE_COUNTER("OnTaskFinished");

		Task->ReportBuildTime();
		if (Task->NewOctree.IsValid()) // Make sure the new octree is valid before using it, else the ids will be out of sync
		{
			// Move Octree to OctreeToDelete so that we delete it async, without a huge cost on the game thread
			ensure(!Task->OctreeToDelete.IsValid());
			Task->OctreeToDelete = MoveTemp(Octree);
			
			Octree = Task->NewOctree;

			INC_DWORD_STAT_BY(STAT_VoxelChunkUpdates, Task->ChunkUpdates.Num());
			Settings.Renderer->UpdateLODs(Octree->UpdateIndex, Task->ChunkUpdates);

			if (Settings.bStaticWorld)
			{
				// Destroy octree and stop ticking
				VOXEL_SCOPE_COUNTER("Destroying octree");
				Octree.Reset();
				Task->NewOctree.Reset();
				Task->OldOctree.Reset();
				StopTicking();
			}
		}
		bAsyncTaskWorking = false;
	}
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelDefaultLODManager::UpdateInvokers()
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!VoxelWorldInterface.IsValid())
	{
		return;
	}
	
	bool bNeedUpdate = false;
	
	TArray<TWeakObjectPtr<UVoxelInvokerComponentBase>> NewInvokerComponents = UVoxelInvokerComponentBase::GetInvokers(Settings.World.Get());
	NewInvokerComponents.Sort([](auto& A, auto& B) { return A.Get() < B.Get(); });

	if (InvokerComponents != NewInvokerComponents)
	{
		bNeedUpdate = true;
	}
	else
	{
		const uint64 Threshold = FMath::Square(FMath::Max(DynamicSettings->InvokerDistanceThreshold / Settings.VoxelSize, 0.f)); // Truncate
		for (auto& Invoker : NewInvokerComponents)
		{
			const FIntVector OldPosition = InvokerComponentsLocalPositions[Invoker];
			const FIntVector NewPosition = Invoker->GetInvokerVoxelPosition(VoxelWorldInterface.Get());

			if (FVoxelUtilities::SquaredSize(OldPosition - NewPosition) > Threshold)
			{
				bNeedUpdate = true;
				break;
			}
		}
	}

	if (bNeedUpdate)
	{
		if (!Settings.bConstantLOD)
		{
			bLODUpdateQueued = true;
		}

		InvokerComponents = NewInvokerComponents;
		InvokerComponentsLocalPositions.Reset();

		for (auto& Invoker : NewInvokerComponents)
		{
			const FIntVector LocalPosition = Invoker->GetInvokerVoxelPosition(VoxelWorldInterface.Get());
			InvokerComponentsLocalPositions.Add(Invoker, LocalPosition);
		}

		TArray<FIntVector> InvokersPositionsForPriorities;
		for (auto& It : InvokerComponentsLocalPositions)
		{
			if (It.Key->bUseForPriorities)
			{
				InvokersPositionsForPriorities.Add(It.Value);
			}
		}
		Settings.Renderer->SetInvokersPositionsForPriorities(InvokersPositionsForPriorities);
	}

	const double Time = FPlatformTime::Seconds();
	if (bLODUpdateQueued && Task->IsDone() && Time - LastLODUpdateTime > Settings.MinDelayBetweenLODUpdates)
	{
		bLODUpdateQueued = false;
		LastLODUpdateTime = Time;
		UpdateLODs();
	}
}

void FVoxelDefaultLODManager::UpdateLODs()
{
	VOXEL_FUNCTION_COUNTER();

	TArray<FVoxelInvoker> Invokers;
	for (auto& It : InvokerComponentsLocalPositions)
	{
		FVoxelInvoker Invoker;
		It.Key->GetInvokerSettings(
			const_cast<AVoxelWorldInterface*>(VoxelWorldInterface.Get()),
			Invoker.bUseForLOD,
			Invoker.LODToSet,
			Invoker.LODBounds,
			Invoker.bUseForCollisions,
			Invoker.CollisionsBounds,
			Invoker.bUseForNavmesh,
			Invoker.NavmeshBounds,
			Invoker.bUseForTessellation,
			Invoker.TessellationBounds);
		Invoker.bUseForLOD &= It.Key->IsLocalInvoker();
		if (Invoker.bUseForLOD || Invoker.bUseForCollisions || Invoker.bUseForNavmesh || Invoker.bUseForTessellation)
		{
			Invokers.Emplace(Invoker);
		}
	}

	FVoxelRenderOctreeSettings OctreeSettings;
	OctreeSettings.MinLOD = DynamicSettings->MinLOD;
	OctreeSettings.MaxLOD = DynamicSettings->MaxLOD;
	OctreeSettings.WorldBounds = Settings.WorldBounds;

	OctreeSettings.Invokers = Invokers;
	OctreeSettings.ChunksCullingLOD = DynamicSettings->ChunksCullingLOD;

	OctreeSettings.bEnableRender = DynamicSettings->bEnableRender;
	OctreeSettings.bEnableTransitions = Settings.bEnableTransitions;
	OctreeSettings.bInvertTransitions = Settings.bInvertTransitions;

	OctreeSettings.bEnableCollisions = DynamicSettings->bEnableCollisions;
	OctreeSettings.bComputeVisibleChunksCollisions = DynamicSettings->bComputeVisibleChunksCollisions;
	OctreeSettings.VisibleChunksCollisionsMaxLOD = DynamicSettings->VisibleChunksCollisionsMaxLOD;

	OctreeSettings.bEnableNavmesh = DynamicSettings->bEnableNavmesh;
	OctreeSettings.bComputeVisibleChunksNavmesh = DynamicSettings->bComputeVisibleChunksNavmesh;
	OctreeSettings.VisibleChunksNavmeshMaxLOD = DynamicSettings->VisibleChunksNavmeshMaxLOD;

	OctreeSettings.bEnableTessellation = DynamicSettings->bEnableTessellation;

	Task->Init(OctreeSettings, Octree);
	Settings.Pool->QueueTask(EVoxelTaskType::RenderOctree, Task.Get());
	bAsyncTaskWorking = true;
}

void FVoxelDefaultLODManager::ClearInvokerComponents()
{
	InvokerComponents.Reset();
}