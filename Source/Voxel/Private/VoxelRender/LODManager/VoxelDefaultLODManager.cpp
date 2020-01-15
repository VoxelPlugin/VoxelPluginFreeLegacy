// Copyright 2020 Phyronnaz

#include "VoxelRender/LODManager/VoxelDefaultLODManager.h"
#include "VoxelRender/LODManager/VoxelRenderOctree.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "IntBox.h"
#include "IVoxelPool.h"
#include "VoxelWorldInterface.h"
#include "VoxelComponents/VoxelInvokerComponent.h"

DECLARE_DWORD_COUNTER_STAT(TEXT("Voxel Chunk Updates"), STAT_VoxelChunkUpdates, STATGROUP_Voxel);

TVoxelSharedRef<FVoxelDefaultLODManager> FVoxelDefaultLODManager::Create(
	const FVoxelLODSettings& LODSettings,
	TWeakObjectPtr<const AVoxelWorldInterface> VoxelWorldInterface,
	const TVoxelSharedRef<FVoxelLODDynamicSettings>& DynamicSettings)
{
	return MakeShareable(
		new FVoxelDefaultLODManager(
			LODSettings,
			VoxelWorldInterface,
			DynamicSettings));
}

FVoxelDefaultLODManager::FVoxelDefaultLODManager(
	const FVoxelLODSettings& LODSettings,
	TWeakObjectPtr<const AVoxelWorldInterface> VoxelWorldInterface,
	const TVoxelSharedRef<FVoxelLODDynamicSettings>& DynamicSettings)
	: IVoxelLODManager(LODSettings) 
	, VoxelWorldInterface(VoxelWorldInterface)
	, DynamicSettings(DynamicSettings)
	, Task(MakeUnique<FVoxelRenderOctreeAsyncBuilder>(LODSettings.OctreeDepth, LODSettings.WorldBounds))
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
	// For normals etc
	return FIntBox(Bounds.Min - FIntVector(2, 2, 2), Bounds.Max + FIntVector(2, 2, 2));
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
	
	TArray<TWeakObjectPtr<UVoxelInvokerComponent>> NewInvokerComponents = UVoxelInvokerComponent::GetInvokers(Settings.World.Get());
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
			const auto& OldPosition = InvokerComponentsLocalPositions[Invoker];
			const auto NewPosition = VoxelWorldInterface->GlobalToLocal(Invoker->GetPosition());

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
			const auto LocalPosition = VoxelWorldInterface->GlobalToLocal(Invoker->GetPosition());
			InvokerComponentsLocalPositions.Add(Invoker, LocalPosition);
		}
		TArray<FIntVector> Array;
		InvokerComponentsLocalPositions.GenerateValueArray(Array);
		Settings.Renderer->SetInvokersPositions(Array);
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
		auto& Invoker = It.Key;
		FVoxelInvoker InvokerSettings;
		InvokerSettings.Position = It.Value;
		InvokerSettings.bUseForLODs = Invoker->bUseForLODs && Invoker->IsLocalInvoker();
		InvokerSettings.bUseForCollisions = Invoker->bUseForCollisions;
		InvokerSettings.SquaredCollisionsRange = GetSquaredDistance(Invoker->CollisionsRange);
		InvokerSettings.bUseForNavmesh = Invoker->bUseForNavmesh;
		InvokerSettings.SquaredNavmeshRange = GetSquaredDistance(Invoker->NavmeshRange);
		Invokers.Emplace(InvokerSettings);
	}

	TArray<uint64> SquaredLODsDistances;
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

	FVoxelRenderOctreeSettings OctreeSettings;
	OctreeSettings.MinLOD = DynamicSettings->MinLOD;
	OctreeSettings.MaxLOD = DynamicSettings->MaxLOD;
	OctreeSettings.WorldBounds = Settings.WorldBounds;
	OctreeSettings.SquaredLODsDistances = SquaredLODsDistances;

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
	OctreeSettings.SquaredTessellationDistance = GetSquaredDistance(DynamicSettings->TessellationDistance);

	Task->Init(OctreeSettings, Octree);
	Settings.Pool->QueueTask(EVoxelTaskType::RenderOctree, Task.Get());
	bAsyncTaskWorking = true;
}

uint64 FVoxelDefaultLODManager::GetSquaredDistance(float DistanceInCm) const
{
	return FMath::Square<uint64>(FMath::CeilToInt(DistanceInCm / Settings.VoxelSize));
}