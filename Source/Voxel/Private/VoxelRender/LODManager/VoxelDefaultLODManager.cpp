// Copyright 2021 Phyronnaz

#include "VoxelRender/LODManager/VoxelDefaultLODManager.h"
#include "VoxelRender/LODManager/VoxelRenderOctree.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelIntBox.h"
#include "VoxelPool.h"
#include "VoxelWorld.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"

#include "EngineUtils.h"
#include "Lightmass/LightmassImportanceVolume.h"

DECLARE_DWORD_COUNTER_STAT(TEXT("Voxel Chunk Updates"), STAT_VoxelChunkUpdates, STATGROUP_VoxelCounters);

static TAutoConsoleVariable<int32> CVarFreezeLODs(
	TEXT("voxel.lod.FreezeLODs"),
	0,
	TEXT("Stops LOD manager tick"),
	ECVF_Default);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DEFINE_VOXEL_SUBSYSTEM_PROXY(UVoxelDefaultLODSubsystemProxy);

void FVoxelDefaultLODManager::Create()
{
	Super::Create();

	Task = MakeVoxelAsyncWork<FVoxelRenderOctreeAsyncBuilder>(Settings.RenderOctreeDepth, Settings.GetWorldBounds());
	UVoxelInvokerComponentBase::OnForceRefreshInvokers.AddThreadSafeSP(this, &FVoxelDefaultLODManager::ClearInvokerComponents);
}

void FVoxelDefaultLODManager::Destroy()
{
	Super::Destroy();
	
	if (IsTicking())
	{
		StopTicking();
	}
	
	if (!Task->IsDone())
	{
		Task->CancelAndAutodelete();
		Task.Release();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline FVoxelIntBox GetBoundsToUpdate(const FVoxelIntBox& Bounds)
{
	// For normals
	return Bounds.Extend(2);
}

int32 FVoxelDefaultLODManager::UpdateBounds(const FVoxelIntBox& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!Octree.IsValid())
	{
		return 0;
	}

	TArray<uint64> ChunksToUpdate;
	Octree->GetChunksToUpdateForBounds(GetBoundsToUpdate(Bounds), ChunksToUpdate, OnChunkUpdate);
	
	return GetSubsystemChecked<IVoxelRenderer>().UpdateChunks(Bounds, ChunksToUpdate, FinishDelegate);
}

int32 FVoxelDefaultLODManager::UpdateBounds(const TArray<FVoxelIntBox>& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!Octree.IsValid() || Bounds.Num() == 0)
	{
		return 0;
	}

	TArray<uint64> ChunksToUpdate;
	FVoxelIntBox GlobalBounds = Bounds[0];
	for (auto& BoundsToUpdate : Bounds)
	{
		GlobalBounds = GlobalBounds + BoundsToUpdate;
		Octree->GetChunksToUpdateForBounds(GetBoundsToUpdate(BoundsToUpdate), ChunksToUpdate, OnChunkUpdate);
	}
	return GetSubsystemChecked<IVoxelRenderer>().UpdateChunks(GlobalBounds, ChunksToUpdate, FinishDelegate);
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDefaultLODManager::Tick(float DeltaTime)
{
	VOXEL_FUNCTION_COUNTER();

	if (CVarFreezeLODs.GetValueOnGameThread() != 0)
	{
		return;
	}
	
	const double Time = FPlatformTime::Seconds();
	if (Time - LastInvokersUpdateTime > Settings.MinDelayBetweenLODUpdates)
	{
		LastInvokersUpdateTime = Time;
		UpdateInvokers();
	}

	if (bAsyncTaskWorking && Task->IsDone())
	{
		VOXEL_SCOPE_COUNTER("OnTaskFinished");
	
		LOG_VOXEL(Verbose, TEXT("LOD Update Finished"));

		Task->ReportBuildTime();
		if (Task->NewOctree.IsValid()) // Make sure the new octree is valid before using it, else the ids will be out of sync
		{
			// Move Octree to OctreeToDelete so that we delete it async, without a huge cost on the game thread
			ensure(!Task->OctreeToDelete.IsValid());
			Task->OctreeToDelete = MoveTemp(Octree);
			
			Octree = Task->NewOctree;

			INC_DWORD_STAT_BY(STAT_VoxelChunkUpdates, Task->ChunkUpdates.Num());
			GetSubsystemChecked<IVoxelRenderer>().UpdateLODs(Octree->UpdateIndex, Task->ChunkUpdates);

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
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDefaultLODManager::UpdateInvokers()
{
	VOXEL_FUNCTION_COUNTER();
	
	auto* VoxelWorld = Settings.VoxelWorld.Get();
	if (!VoxelWorld)
	{
		return;
	}
	ensure(SortedInvokerComponents.Num() == InvokerComponentsInfos.Num());

	if (Settings.bContributesToStaticLighting)
	{
		// For static lighting, we need to fixup lightmass importance volumes
		auto* World = VoxelWorld->GetWorld();
		for (TActorIterator<ALightmassImportanceVolume> It(World); It; ++It)
		{
			auto* Volume = *It;
			if (!Volume->FindComponentByClass(UVoxelVolumeInvokerComponent::StaticClass()))
			{
				auto* Component = NewObject<UVoxelVolumeInvokerComponent>(Volume, NAME_None, RF_Transient);
				Component->bEditorOnlyInvoker = true;
				Component->AttachToComponent(Volume->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				Component->RegisterComponent();
				
				checkVoxelSlow(Volume->FindComponentByClass(UVoxelVolumeInvokerComponent::StaticClass()));
			}
		}
	}
	
	bool bNeedUpdate = false;
	
	TArray<TWeakObjectPtr<UVoxelInvokerComponentBase>> NewSortedInvokerComponents = UVoxelInvokerComponentBase::GetInvokers(VoxelWorld);
	NewSortedInvokerComponents.Sort([](auto& A, auto& B) { return A.Get() < B.Get(); });
	
	if (SortedInvokerComponents != NewSortedInvokerComponents)
	{
		LOG_VOXEL(Verbose, TEXT("Tiggering LOD Update: Invoker Components Array changed"));
		bNeedUpdate = true;
	}

	TMap<TWeakObjectPtr<UVoxelInvokerComponentBase>, FVoxelInvokerInfo> NewInvokerComponentsInfos;
	NewInvokerComponentsInfos.Reserve(NewSortedInvokerComponents.Num());
	
	const uint64 SquaredDistanceThreshold = FMath::Square(FMath::Max(DynamicSettings->InvokerDistanceThreshold / Settings.VoxelSize, 0.f)); // Truncate
	for (const auto& InvokerComponent : NewSortedInvokerComponents)
	{
		FVoxelInvokerSettings InvokerSettings = InvokerComponent->GetInvokerSettings(VoxelWorld);
		InvokerSettings.bUseForLOD &= InvokerComponent->IsLocalInvoker();

		const FIntVector InvokerPosition = InvokerComponent->GetInvokerVoxelPosition(VoxelWorld);
		
		FVoxelInvokerInfo Info;
		Info.LocalPosition = InvokerPosition;
		Info.Settings = InvokerSettings;

		NewInvokerComponentsInfos.Add(InvokerComponent, Info);

		if (!bNeedUpdate)
		{
			auto* ExistingInfo = InvokerComponentsInfos.Find(InvokerComponent);
			if (ensure(ExistingInfo)) // Should be valid if bNeedUpdate is false (would be set to true if invoker components array changed)
			{
				const auto& OldSettings = ExistingInfo->Settings;
				const auto& NewSettings = Info.Settings;
				if (OldSettings.bUseForLOD != NewSettings.bUseForLOD)
				{
					LOG_VOXEL(Verbose, TEXT("Tiggering LOD Update: bUseForLOD changed"));
					bNeedUpdate = true;
				}
				else if (OldSettings.LODToSet != NewSettings.LODToSet)
				{
					LOG_VOXEL(Verbose, TEXT("Tiggering LOD Update: LODToSet changed"));
					bNeedUpdate = true;
				}
				else if (OldSettings.bUseForCollisions != NewSettings.bUseForCollisions)
				{
					LOG_VOXEL(Verbose, TEXT("Tiggering LOD Update: bUseForCollisions changed"));
					bNeedUpdate = true;
				}
				else if (OldSettings.bUseForNavmesh != NewSettings.bUseForNavmesh)
				{
					LOG_VOXEL(Verbose, TEXT("Tiggering LOD Update: bUseForNavmesh changed"));
					bNeedUpdate = true;
				}
				else if (FVoxelUtilities::SquaredSize(ExistingInfo->LocalPosition - Info.LocalPosition) > SquaredDistanceThreshold)
				{
					LOG_VOXEL(Verbose, TEXT("Tiggering LOD Update: Invoker Component moved"));
					bNeedUpdate = true;
				}
			}
		}
	}

	if (bNeedUpdate)
	{
		if (!Settings.bConstantLOD)
		{
			bLODUpdateQueued = true;
		}

		SortedInvokerComponents = MoveTemp(NewSortedInvokerComponents);
		InvokerComponentsInfos = MoveTemp(NewInvokerComponentsInfos);

		// Update the invoker positions, used for priorities
		TArray<FIntVector> NewInvokerPositions;
		for (auto& It : InvokerComponentsInfos)
		{
			if (It.Key->bUseForPriorities)
			{
				NewInvokerPositions.Add(It.Value.LocalPosition);
			}
		}
		
		auto& InvokerPositions = RuntimeData->InvokersPositionsForPriorities;
		if (InvokerPositions->GetMax() < NewInvokerPositions.Num())
		{
			InvokerPositions = MakeVoxelShared<FInvokerPositionsArray>(2 * NewInvokerPositions.Num());
		}
		InvokerPositions->Set(NewInvokerPositions);
	}

	ensure(SortedInvokerComponents.Num() == InvokerComponentsInfos.Num());

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
	
	LOG_VOXEL(Verbose, TEXT("Starting LOD Update"));

	FVoxelRenderOctreeSettings OctreeSettings;
	OctreeSettings.MinLOD = DynamicSettings->MinLOD;
	OctreeSettings.MaxLOD = DynamicSettings->MaxLOD;
	OctreeSettings.WorldBounds = Settings.GetWorldBounds();

	OctreeSettings.Invokers.Reserve(InvokerComponentsInfos.Num());
	for (const auto& It : InvokerComponentsInfos)
	{
		if (It.Value.Settings.bUseForLOD ||
			It.Value.Settings.bUseForCollisions ||
			It.Value.Settings.bUseForNavmesh)
		{
			OctreeSettings.Invokers.Add(It.Value.Settings);
		}
	}

	OctreeSettings.ChunksCullingLOD = DynamicSettings->ChunksCullingLOD;

	OctreeSettings.bEnableRender = DynamicSettings->bRenderWorld;
	OctreeSettings.bEnableTransitions = Settings.bEnableTransitions;
	OctreeSettings.bInvertTransitions = Settings.RenderType == EVoxelRenderType::SurfaceNets;

	OctreeSettings.bEnableCollisions = DynamicSettings->bEnableCollisions;
	OctreeSettings.bComputeVisibleChunksCollisions = DynamicSettings->bComputeVisibleChunksCollisions;
	OctreeSettings.VisibleChunksCollisionsMaxLOD = DynamicSettings->VisibleChunksCollisionsMaxLOD;

	OctreeSettings.bEnableNavmesh = DynamicSettings->bEnableNavmesh;
	OctreeSettings.bComputeVisibleChunksNavmesh = DynamicSettings->bComputeVisibleChunksNavmesh;
	OctreeSettings.VisibleChunksNavmeshMaxLOD = DynamicSettings->VisibleChunksNavmeshMaxLOD;

	Task->Init(OctreeSettings, Octree);
	GetSubsystemChecked<FVoxelPool>().QueueTask(Task.Get());
	bAsyncTaskWorking = true;
}

void FVoxelDefaultLODManager::ClearInvokerComponents()
{
	SortedInvokerComponents.Reset();
	InvokerComponentsInfos.Reset();
}