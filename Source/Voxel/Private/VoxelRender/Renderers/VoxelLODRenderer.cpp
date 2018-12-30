// Copyright 2018 Phyronnaz

#include "VoxelMCRenderer.h"
#include "VoxelLogStatDefinitions.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelData.h"
#include "VoxelChunkOctree.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "Misc/QueuedThreadPool.h"
#include "DrawDebugHelpers.h"
#include "VoxelPoolManager.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "VoxelRender/VoxelRenderUtilities.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"

DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::ScheduleTick"), STAT_LODVoxelRender_ScheduleTick, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateBox.RemoveOldChunks"), STAT_LODVoxelRender_UpdateBox_RemoveOldChunks, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::Tick"), STAT_LODVoxelRender_Tick, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::Tick.CheckTaskGroups"), STAT_LODVoxelRender_Tick_CheckTaskGroups, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::Tick.TickChunks"), STAT_LODVoxelRender_Tick_TickChunks, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::Tick.ChunkToRemove"), STAT_LODVoxelRender_Tick_ChunkToRemove, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::Tick.CheckIsDoneOctreeBuilder"), STAT_LODVoxelRender_Tick_CheckIsDoneOctreeBuilder, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::Tick.Update"), STAT_LODVoxelRender_Tick_Update, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateLOD"), STAT_LODVoxelRender_UpdateLOD, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateLOD.ChunksToDelete"), STAT_LODVoxelRender_UpdateLOD_ChunksToDelete, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateLOD.ChunksToCreate"), STAT_LODVoxelRender_UpdateLOD_ChunksToCreate, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateLOD.ChunksToCreate.FindOldChunks"), STAT_LODVoxelRender_UpdateLOD_ChunksToCreate_FindOldChunks, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateLOD.ChunksToCreate.UpdateChunks"), STAT_LODVoxelRender_UpdateLOD_ChunksToCreate_UpdateChunks, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateLOD.UpdateTransitions"), STAT_LODVoxelRender_UpdateLOD_UpdateTransitions, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateLOD.ChunksToDelete.CancelTicks"), STAT_LODVoxelRender_UpdateLOD_ChunksToDelete_CancelTicks, STATGROUP_Voxel);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FAsyncOctreeBuilderTask::FAsyncOctreeBuilderTask(uint8 LOD, const FIntBox& WorldBounds)
	: LOD(LOD)
	, WorldBounds(WorldBounds)
{
}

void FAsyncOctreeBuilderTask::Init(
	const TArray<FIntVector>& InCameraPositions, 
	TSharedPtr<FVoxelChunkOctree> InOctree, 
	const TMap<uint8, float>& InLODToMinDistance,
	float InVoxelSize,
	uint8 InLODLimit)
{
	check(!bIsActive);

	CameraPositions = InCameraPositions;
	Octree = InOctree;
	LODToMinDistance = InLODToMinDistance;
	VoxelSize = InVoxelSize;
	LODLimit = InLODLimit;

	bIsActive = true;
}

void FAsyncOctreeBuilderTask::Reset()
{
	bIsActive = false;
	IsDoneCounter.Reset();
}

void FAsyncOctreeBuilderTask::DoWork()
{
	check(bIsActive);

	ChunksToDelete.Reset();
	ChunksToCreate.Reset();
	NewOctree.Reset();
	OldOctree.Reset();
	TransitionsMasks.Reset();

	FVoxelChunkOctreeSettings Settings;
	Settings.LODLimit = LODLimit;
	Settings.WorldBounds = WorldBounds;
	Settings.CameraPositions = CameraPositions;
	Settings.SquaredDistances.SetNum(MAX_WORLD_DEPTH);
	for (int Index = 1; Index < MAX_WORLD_DEPTH; Index++)
	{
		// +1: We want to divide LOD 2 if LOD _1_ min distance isn't met, not if LOD _2_ min distance isn't met
		int CurrentLOD = Index - 1;
		while (!LODToMinDistance.Contains(CurrentLOD))
		{
			check(CurrentLOD > 0);
			CurrentLOD--;
		}
		Settings.SquaredDistances[Index] = FMath::Square<uint64>(FMath::CeilToInt(LODToMinDistance[CurrentLOD] / VoxelSize));
	}

	NewOctree = MakeShared<FVoxelChunkOctree>(&Settings, LOD);
	OldOctree = Octree;

	TSet<FIntBox> OldBounds;
	TSet<FIntBox> NewBounds;
	TMap<FIntBox, uint8> OldTransitionsMasks;

	if (OldOctree.IsValid())
	{
		OldOctree->GetLeavesBounds(OldBounds);
		OldOctree->GetLeavesTransitionsMasks(OldTransitionsMasks);
	}
	NewOctree->GetLeavesBounds(NewBounds);

	ChunksToDelete = OldBounds.Difference(NewBounds);
	ChunksToCreate = NewBounds.Difference(OldBounds);

	NewOctree->GetLeavesTransitionsMasks(TransitionsMasks);
	for (auto It = OldTransitionsMasks.CreateIterator(); It; ++It)
	{
		uint8* Value = TransitionsMasks.Find(It.Key());
		if (Value && It.Value() == *Value)
		{
			TransitionsMasks.Remove(It.Key());
		}
	}

	check(bIsActive);
}

void FAsyncOctreeBuilderTask::DoThreadedWork()
{
	DoWork();

	FScopeLock Lock(&DoneSection);
	IsDoneCounter.Increment();

	if (bAutodelete)
	{
		delete this;
	}
}

void FAsyncOctreeBuilderTask::Abandon()
{
	
}

bool FAsyncOctreeBuilderTask::IsDone() const
{
	return IsDoneCounter.GetValue() > 0;
}

void FAsyncOctreeBuilderTask::Autodelete()
{
	FScopeLock Lock(&DoneSection);
	bAutodelete = true;

	if (IsDone())
	{
		delete this;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelChunkToDelete::FVoxelChunkToDelete(FVoxelRenderChunk& OldChunk)
	: Render(OldChunk.Render)
	, Bounds(OldChunk.GetBounds())
	, OldChunks(OldChunk.GetPreviousChunks())
	, Mesh(OldChunk.GetMesh())
{
}

FVoxelChunkToDelete::~FVoxelChunkToDelete()
{
	if (Mesh)
	{
		Render->RemoveMesh(Mesh, Bounds);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelLODRenderer::FVoxelLODRenderer(AVoxelWorld* World)
	: IVoxelRender(World)
	, Pool(World->GetPool())
	, OctreeBuilder(MakeUnique<FAsyncOctreeBuilderTask>(World->GetOctreeDepth(), World->GetData()->GetBounds()))
{
}

FVoxelLODRenderer::~FVoxelLODRenderer()
{
	for (auto& Chunk : Chunks)
	{
		Chunk.Value->Destroy();
	}

	if (OctreeBuilder->IsActive() && !Pool->OctreeBuilderPool->RetractQueuedWork(OctreeBuilder.Get()))
	{
		OctreeBuilder->Autodelete();
		OctreeBuilder.Release();
	}
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelLODRenderer::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_Tick);
	
	WorldTime += DeltaTime;

	const float Time = GetWorldTime();

	if (World->bShowAllRenderChunks)
	{
		for (auto& It : Chunks)
		{
			World->DrawDebugIntBox(It.Value->GetBounds(), 2 * DeltaTime, 100, FColor::Red);
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_Tick_CheckTaskGroups);

		for (const auto& Group : DependenciesHandler.GetTaskGroups())
		{
			bool bAllFinished = true;
			for (auto& Task : Group->Tasks)
			{
				if (Task.Chunk.IsValid())
				{
					auto PinnedChunk = Task.Chunk.Pin();
					if (!PinnedChunk->IsDone() || PinnedChunk->GetTaskId() != Task.Id)
					{
						bAllFinished = false;
						break;
					}
				}
			}
			if (bAllFinished)
			{
				for (auto& Task : Group->Tasks)
				{
					if (Task.Chunk.IsValid())
					{
						Task.Chunk.Pin()->ScheduleTick();
					}
				}
				DependenciesHandler.ClearGroup(Group);
			}
		}
		DependenciesHandler.CleanGroups();
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_Tick_ChunkToRemove);

		for (auto& Chunk : ChunkToRemove)
		{
			if (Chunk.TimeOfDeletion < Time)
			{
				Chunk.Mesh->ClearSections();
				
				if (Chunk.bCollisions)
				{
					InactiveMeshesCollisions.Push(Chunk.Mesh);
				}
				else
				{
					InactiveMeshesNoCollisions.Push(Chunk.Mesh);
				}
			}
		}
		ChunkToRemove.RemoveAll([&](auto& Chunk) { return Chunk.TimeOfDeletion < Time; });
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_Tick_TickChunks);

		TSet<FVoxelRenderChunk*> ChunksToTickCopy;
		{
			FScopeLock Lock(&ChunksToTickLock);
			ChunksToTickCopy = ChunksToTick;
			ChunksToTick.Reset();
		}

		for (auto& Chunk : ChunksToTickCopy)
		{
			Chunk->Tick();
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_Tick_CheckIsDoneOctreeBuilder);

		if (OctreeBuilder->IsActive() && OctreeBuilder->IsDone())
		{
			// For the first finished octree update (else the task count is 0 while the octree is building)
			bOctreeBuilt = true;
			UpdateLOD();
			OctreeBuilder->Reset();
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_Tick_Update);

		if (NextUpdateTime < Time && !OctreeBuilder->IsActive())
		{
			NextUpdateTime = Time + 1.f / World->GetLODUpdateRate();

			TArray<FIntVector> CameraPositions;
			for (const auto& Invoker : World->GetInvokers())
			{
				if (Invoker.IsValid() && (Invoker->IsLocalInvoker() || World->IsDedicatedServer()))
				{
					CameraPositions.Add(World->GlobalToLocal(Invoker->GetPosition()));
				}
			}
			OctreeBuilder->Init(
				CameraPositions,
				Octree,
				World->GetLODToMinDistance(),
				World->GetVoxelSize(),
				World->GetLODLimit());
			Pool->OctreeBuilderPool->AddQueuedWork(OctreeBuilder.Get());
		}
	}
}

void FVoxelLODRenderer::UpdateBoxInternal(const FIntBox& Box, bool bRemoveHoles, TFunction<void()> CallbackWhenUpdated)
{
	if (Octree.IsValid())
	{
		TArray<FVoxelChunkOctree*> Leaves;
		Octree->GetLeavesOverlappingBox(Box, Leaves);
		
		TArray<FIntBox> DebugBounds;

		if (bRemoveHoles && World->GetWaitForOtherChunksToAvoidHoles())
		{
			TArray<FVoxelTasksDependenciesHandler::FLockedTask> Tasks;
			for (auto& Chunk : Leaves)
			{
				auto* RenderChunkPtr = Chunks.Find(Chunk->GetBounds());
				if (!RenderChunkPtr) { continue; } // If culled		
				auto& RenderChunk = *RenderChunkPtr;

				uint64 Id = RenderChunk->GetQueuedTaskId();
				if (Id == 0)
				{
					Id = DependenciesHandler.GetTaskId();
				}
				Tasks.Emplace(RenderChunk, Id);
				
				check(RenderChunk->Bounds.Intersect(Box));
				RenderChunk->UpdateChunk(Id);
				DebugBounds.Add(RenderChunk->Bounds);
			}
			DependenciesHandler.AddGroup(MoveTemp(Tasks), CallbackWhenUpdated);
		}
		else
		{
			for (auto& Chunk : Leaves)
			{
				auto* RenderChunkPtr = Chunks.Find(Chunk->GetBounds());
				if (!RenderChunkPtr) { continue; } // If culled
				auto& RenderChunk = *RenderChunkPtr;
				check(RenderChunk->Bounds.Intersect(Box));
				RenderChunk->UpdateChunk(RenderChunk->GetQueuedTaskId());
				DebugBounds.Add(RenderChunk->Bounds);
			}
		}

		if (World->bShowUpdatedChunks)
		{
			for (auto& DebugBound : DebugBounds)
			{
				World->DrawDebugIntBox(DebugBound, 0.5, World->UpdatedChunksThickness, FLinearColor::Blue);
			}
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_UpdateBox_RemoveOldChunks);

		for (auto& Mesh : ChunkToRemove)
		{
			if (Mesh.Bounds.Intersect(Box))
			{
				Mesh.TimeOfDeletion = -1;
				if (Octree.IsValid())
				{
					TArray<FVoxelChunkOctree*> Leaves;
					Octree->GetLeavesOverlappingBox(Mesh.Bounds, Leaves);
					for (auto& Chunk : Leaves)
					{
						auto* RenderChunkPtr = Chunks.Find(Chunk->GetBounds());
						if (RenderChunkPtr)
						{
							// If not culled
							(*RenderChunkPtr)->ResetAlpha();
						}
					}
				}
			}
		}
	}
}

uint8 FVoxelLODRenderer::GetLODAtPosition(const FIntVector& Position) const
{
	return Octree->GetLeaf(Position)->LOD;
}

void FVoxelLODRenderer::RecreateMaterials()
{
	for (auto& Chunk : Chunks)
	{
		Chunk.Value->RecreateMaterials();
	}
}

void FVoxelLODRenderer::RecomputeMeshPositions()
{
	for (auto& Chunk : Chunks)
	{
		Chunk.Value->RecomputeMeshPosition();
	}
	for (auto& Chunk : ChunkToRemove)
	{
		Chunk.Mesh->SetRelativeLocation(World->GetChunkRelativePosition(Chunk.Bounds.Min));
	}
}

void FVoxelLODRenderer::SetScalarParameterValue(FName ParameterName, float Value)
{
	for (auto& Chunk : Chunks)
	{
		Chunk.Value->SetScalarParameterValue(ParameterName, Value);
	}
}

void FVoxelLODRenderer::SetTextureParameterValue(FName ParameterName, UTexture* Value)
{
	for (auto& Chunk : Chunks)
	{
		Chunk.Value->SetTextureParameterValue(ParameterName, Value);
	}
}

void FVoxelLODRenderer::SetVectorParameterValue(FName ParameterName, FLinearColor Value)
{
	for (auto& Chunk : Chunks)
	{
		Chunk.Value->SetVectorParameterValue(ParameterName, Value);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelLODRenderer::RemoveMesh(UVoxelProceduralMeshComponent* Mesh, const FIntBox& Bounds)
{
	FVoxelChunkToRemove Chunk;
	Chunk.Mesh = Mesh;
	Chunk.TimeOfDeletion = GetWorldTime() + 2 * World->GetChunksFadeDuration();
	Chunk.bCollisions = Mesh->GetCollisionEnabled() != ECollisionEnabled::NoCollision;
	Chunk.Bounds = Bounds;
	
	for (int Index = 0; Index < Mesh->GetNumSections(); Index++)
	{
		UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Mesh->GetProcMeshSection(Index)->Material);
		if (Material)
		{
			Material->SetScalarParameterValue(FName(TEXT("EndTime")), FVoxelRenderUtilities::GetWorldCurrentTime(Mesh->GetWorld()) + World->GetChunksFadeDuration() * 2);
		}
	}
	ChunkToRemove.Add(Chunk);
}

UVoxelProceduralMeshComponent* FVoxelLODRenderer::GetNewMesh(const FIntVector& Position, uint8 LOD)
{
	auto WorldType = World->GetWorld()->WorldType;
	bool bInEditor = WorldType == EWorldType::Editor || WorldType == EWorldType::EditorPreview;
	bool bCollisions = bInEditor ? LOD < 5 : LOD <= World->GetMaxCollisionsLOD();

	UVoxelProceduralMeshComponent* NewMesh;
	if ((bCollisions ? InactiveMeshesCollisions : InactiveMeshesNoCollisions).Num() > 0)
	{
		// Create only if needed
		NewMesh = (bCollisions ? InactiveMeshesCollisions : InactiveMeshesNoCollisions).Pop();
	}
	else
	{
		NewMesh = NewObject<UVoxelProceduralMeshComponent>(World, World->GetProcMeshClass(), NAME_None, RF_Transient);
		NewMesh->SetupAttachment(World->GetRootComponent(), NAME_None);
		NewMesh->RegisterComponent();
		NewMesh->SetRelativeScale3D(FVector::OneVector * World->GetVoxelSize());
		NewMesh->SetCollisionEnabled(bCollisions ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		if (bCollisions && !bInEditor)
		{
			NewMesh->BodyInstance = World->GetCollisionPresets();
		}
	}

	NewMesh->SetRelativeLocation(World->GetChunkRelativePosition(Position));
	NewMesh->InitChunk(LOD, Position);

	return NewMesh;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelLODRenderer::ScheduleTick(FVoxelRenderChunk* Chunk)
{
	SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_ScheduleTick);
	{
		FScopeLock Lock(&ChunksToTickLock);
		ChunksToTick.Add(Chunk);
	}
}

void FVoxelLODRenderer::AddQueuedWork(IVoxelQueuedWork* QueuedWork)
{
	Pool->MeshPool->AddQueuedWork(QueuedWork);
}

bool FVoxelLODRenderer::RetractQueuedWork(IVoxelQueuedWork* QueuedWork)
{
	return Pool->MeshPool->RetractQueuedWork(QueuedWork);
}

uint64 FVoxelLODRenderer::GetSquaredDistanceToInvokers(const FIntVector& Position) const
{
	uint64 Distance = MAX_uint64;
	for (auto& Invoker : World->GetInvokers())
	{
		if (Invoker.IsValid())
		{
			Distance = FMath::Min<uint64>(Distance, FVoxelIntVector::SquaredSize(Position - World->GlobalToLocal(Invoker->GetPosition())));
		}
	}
	return Distance;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelLODRenderer::UpdateLOD()
{
	SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_UpdateLOD);

	check(OctreeBuilder->IsDone());

	TSharedPtr<FVoxelChunkOctree>& OldOctree = OctreeBuilder->OldOctree;
	TSharedPtr<FVoxelChunkOctree>& NewOctree = OctreeBuilder->NewOctree;
	Octree = NewOctree;

	TSet<FIntBox>& ChunksToDelete = OctreeBuilder->ChunksToDelete;
	TSet<FIntBox>& ChunksToCreate = OctreeBuilder->ChunksToCreate;

	TMap<FIntBox, uint8>& TransitionsMasks = OctreeBuilder->TransitionsMasks;

	TMap<FIntBox, TSharedRef<FVoxelChunkToDelete>> ChunksToDeleteMap;

	{
		SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_UpdateLOD_ChunksToDelete);

		TArray<TSharedPtr<FVoxelRenderChunk>> ChunksToCancelTick;
		for (auto& Box : ChunksToDelete)
		{
			TSharedPtr<FVoxelRenderChunk> Chunk;
			if (!Chunks.RemoveAndCopyValue(Box, Chunk))
			{
				continue; // Chunk was culled
			}

			if (Chunk->GetMesh() || Chunk->GetPreviousChunks().Num() > 0)
			{
				ChunksToDeleteMap.Add(Box, MakeShareable(new FVoxelChunkToDelete(*Chunk)));
			}

			Chunk->Destroy();

			ChunksToCancelTick.Add(Chunk);
		}

		{
			SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_UpdateLOD_ChunksToDelete_CancelTicks);

			{
				FScopeLock Lock(&ChunksToTickLock);
				for (auto& Chunk : ChunksToCancelTick)
				{
					ChunksToTick.Remove(Chunk.Get());
				}
			}
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_UpdateLOD_ChunksToCreate);

		for (auto& NewChunkBounds : ChunksToCreate)
		{
			int LOD = FMath::RoundToInt(FMath::Log2(NewChunkBounds.Size().X / CHUNK_SIZE));
			if (World->GetChunksCullingLOD() < LOD || (World->IsDedicatedServer() && LOD != 0))
			{
				continue;
			}
			TSharedRef<FVoxelRenderChunk> NewChunk = GetRenderChunk(this, LOD, NewChunkBounds);

			if (OldOctree.IsValid())
			{
				SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_UpdateLOD_ChunksToCreate_FindOldChunks);

				TArray<FVoxelChunkOctree*> OldChunks;
				OldOctree->GetLeavesOverlappingBox(NewChunkBounds, OldChunks);

				for (auto& OldChunk : OldChunks)
				{
					auto ChunkToDelete = ChunksToDeleteMap.Find(OldChunk->GetBounds());
					if (ChunkToDelete) 
					{
						NewChunk->AddPreviousChunk(*ChunkToDelete);
					}
				}
			}

			{
				SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_UpdateLOD_ChunksToCreate_UpdateChunks);
				NewChunk->UpdateTransitions(TransitionsMasks[NewChunk->Bounds]); // Must be done before UpdateChunk, else an invalid transition mask is used (0)
				NewChunk->UpdateChunk(0);
			}

			//check(!Chunks.Contains(NewChunkBounds));
			Chunks.Add(NewChunkBounds, NewChunk);
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_UpdateLOD_UpdateTransitions);

		for (auto It = TransitionsMasks.CreateIterator(); It; ++It)
		{
			auto* Chunk = Chunks.Find(It.Key());
			if (Chunk)
			{
				(*Chunk)->UpdateTransitions(It.Value());
			}
		}
	}
}

float FVoxelLODRenderer::GetWorldTime() const
{
	// Validity check as it can crash sometimes when closing UE
	float Time = World->GetWorld() ? World->GetWorld()->GetTimeSeconds() : 0;
	if (Time == 0.f)
	{
		Time = WorldTime;
	}
	return Time;
}
