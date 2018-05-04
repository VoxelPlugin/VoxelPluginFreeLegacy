// Copyright 2018 Phyronnaz

#include "LODVoxelRender.h"
#include "VoxelPrivate.h"
#include "VoxelWorld.h"
#include "VoxelData.h"
#include "VoxelChunkOctree.h"
#include "VoxelInvokerComponent.h"
#include "VoxelProceduralMeshComponent.h"
#include "QueuedThreadPool.h"
#include "DrawDebugHelpers.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::Tick"), STAT_LODVoxelRender_Tick, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateLOD"), STAT_LODVoxelRender_UpdateLOD, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateLOD.ChunksToDelete"), STAT_LODVoxelRender_UpdateLOD_ChunksToDelete, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateLOD.ChunksToCreate"), STAT_LODVoxelRender_UpdateLOD_ChunksToCreate, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateLOD.ChunksToCreate.FindOldChunks"), STAT_LODVoxelRender_UpdateLOD_ChunksToCreate_FindOldChunks, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateLOD.ChunksToCreate.UpdateChunks"), STAT_LODVoxelRender_UpdateLOD_ChunksToCreate_UpdateChunks, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FLODVoxelRender::UpdateLOD.UpdateTransitions"), STAT_LODVoxelRender_UpdateLOD_UpdateTransitions, STATGROUP_Voxel);

FAsyncOctreeBuilderTask::FAsyncOctreeBuilderTask(const TArray<FIntBox>& CameraBounds, uint8 LOD, uint8 LODLimit, TSharedPtr<FVoxelChunkOctree> Octree)
	: CameraBounds(CameraBounds)
	, LOD(LOD)
	, LODLimit(LODLimit)
	, Octree(Octree)
{

}

void FAsyncOctreeBuilderTask::DoWork()
{
	NewOctree = MakeShared<FVoxelChunkOctree>(CameraBounds, LOD, LODLimit);
	OldOctree = Octree;

	TSet<FIntBox> OldBounds;
	TSet<FIntBox> NewBounds;

	if (OldOctree.IsValid())
	{
		OldOctree->GetLeavesBounds(OldBounds);
	}
	NewOctree->GetLeavesBounds(NewBounds);

	ChunksToDelete = OldBounds.Difference(NewBounds);
	ChunksToCreate = NewBounds.Difference(OldBounds);

	NewOctree->GetLeavesTransitionsMasks(TransitionsMasks);
}

bool FAsyncOctreeBuilderTask::CanAbandon()
{
	return true;
}

void FAsyncOctreeBuilderTask::Abandon()
{

}

///////////////////////////////////////////////////////////////////////////////

FVoxelRenderChunk::FVoxelRenderChunk(FLODVoxelRender* Render, const FIntVector& Position, uint8 LOD, const FIntBox& Bounds)
	: Render(Render)
	, Position(Position)
	, LOD(LOD)
	, Bounds(Bounds)
	, Mesh(nullptr)
	, TransitionsMask(0)
	, TransitionsDisplayedMask(0)
	, TransitionsCurrentMask(0xFF)
{
	IntermediateChunks.SetNum(SECTIONS_PER_CHUNK);
	InitialUpdatingFinished.SetNum(SECTIONS_PER_CHUNK);
	Tasks.SetNum(SECTIONS_PER_CHUNK);
	SectionNeedsUpdate.SetNum(SECTIONS_PER_CHUNK);
	SectionsBounds.SetNumUninitialized(SECTIONS_PER_CHUNK);
	ChunksCurrentMask.SetNumUninitialized(SECTIONS_PER_CHUNK);

	for (int I = 0; I < CHUNK_MULTIPLIER; I++)
	{
		for (int J = 0; J < CHUNK_MULTIPLIER; J++)
		{
			for (int K = 0; K < CHUNK_MULTIPLIER; K++)
			{
				FIntVector Min = Position + FIntVector(I, J, K) * SectionSize();
				FIntVector Max = Min + FIntVector(SectionSize() + 1, SectionSize() + 1, SectionSize() + 1);

				SectionsBounds[I + CHUNK_MULTIPLIER * J + CHUNK_MULTIPLIER * CHUNK_MULTIPLIER * K] = FIntBox(Min, Max);

				ChunksCurrentMask[I + CHUNK_MULTIPLIER * J + CHUNK_MULTIPLIER * CHUNK_MULTIPLIER * K] = 0xFF;
			}
		}
	}
}

void FVoxelRenderChunk::AddPreviousChunk(const TSharedRef<FVoxelChunkToDelete>& Chunk)
{
	PreviousChunks.Add(Chunk);
	Chunk->AddRef();
}

void FVoxelRenderChunk::EndTasks()
{
	for (auto& Task : Tasks)
	{
		if (Task.IsValid() && !Render->MeshThreadPool->RetractQueuedWork(Task.Get()) && !Task->IsDone())
		{
			Render->AddTaskToDelete(Task);
		}
	}
	if (TransitionsTask.IsValid() && !Render->MeshThreadPool->RetractQueuedWork(TransitionsTask.Get()) && !TransitionsTask->IsDone())
	{
		Render->AddTaskToDelete(TransitionsTask);
	}
}


void FVoxelRenderChunk::UpdateChunk(const FIntBox& Box)
{
	for (int Index = 0; Index < SectionsBounds.Num(); Index++)
	{
		if (SectionsBounds[Index].Intersect(Box))
		{
			if (Tasks[Index].IsValid())
			{
				SectionNeedsUpdate[Index] = true;
			}
			else
			{
				UpdateSection(Index);
			}
		}
	}
}

void FVoxelRenderChunk::UpdateTransitions(uint8 NewTransitionsMask)
{
	if (TransitionsMask != NewTransitionsMask)
	{
		TransitionsMask = NewTransitionsMask;
	}
}

void FVoxelRenderChunk::UpdateTransitions()
{
	if (!TransitionsTask.IsValid())
	{
		TransitionsTask = MakeShared<FAsyncPolygonizerForTransitionsWork>(LOD, Render->World->GetData(), Position, TransitionsMask);
		Render->MeshThreadPool->AddQueuedWork(TransitionsTask.Get());
		TransitionsCurrentMask = TransitionsMask;
	}
}

void FVoxelRenderChunk::Tick()
{
	for (int Index = 0; Index < SECTIONS_PER_CHUNK; Index++)
	{
		auto& Task = Tasks[Index];
		if (Task.IsValid())
		{
			if (Task->IsDone())
			{
				IntermediateChunks[Index] = Task->Chunk;
				const FIntVector& ChunkPosition = Task->ChunkPosition;

				// Mesh
				{
					if (!Mesh && IntermediateChunks[Index].VertexBuffer.Num() != 0)
					{
						Mesh = Render->GetNewMesh(Position, ComputeCollisions(), PreviousChunks.Num() > 0, LOD);
					}

					if (Mesh)
					{
						FVoxelProcMeshSection Section;
						IntermediateChunks[Index].InitSectionBuffers(Section.ProcVertexBuffer, Section.ProcIndexBuffer, TransitionsDisplayedMask);
						ChunksCurrentMask[Index] = TransitionsDisplayedMask;
						Section.SectionLocalBox = FBox(-FVector::OneVector, (TotalSize() + 2) * FVector::OneVector);
						Section.bEnableCollision = ComputeCollisions();

						Mesh->SetProcMeshSection(Index, Section);
					}
				}
				

				Task.Reset();

				if (PreviousChunks.Num() > 0 && !InitialUpdatingFinished[Index])
				{
					InitialUpdatingFinished[Index] = true;
					bool bAllFinished = true;
					for (auto& Value : InitialUpdatingFinished)
					{
						bAllFinished = bAllFinished && Value;
					}
					if (bAllFinished)
					{
						for (auto& PreviousChunk : PreviousChunks)
						{
							PreviousChunk->RemoveRef();
						}
						PreviousChunks.Reset();
					}
				}
			}
		}
		else
		{
			if (SectionNeedsUpdate[Index])
			{
				UpdateSection(Index);
			}
		}
	}

	if (TransitionsTask.IsValid() && TransitionsTask->IsDone())
	{
		if (!Mesh && TransitionsTask->VertexBuffer.Num() != 0)
		{
			Mesh = Render->GetNewMesh(Position, ComputeCollisions(), PreviousChunks.Num() > 0, LOD);
		}

		if (Mesh)
		{
			FVoxelProcMeshSection Section;
			Section.SectionLocalBox = FBox(-FVector::OneVector, (TotalSize() + 2) * FVector::OneVector);
			Section.ProcIndexBuffer = TransitionsTask->IndexBuffer;
			Section.ProcVertexBuffer = TransitionsTask->VertexBuffer;
			Mesh->SetProcMeshSection(SECTIONS_PER_CHUNK, Section);
			TransitionsDisplayedMask = TransitionsCurrentMask;
		}

		TransitionsTask.Reset();
	}

	if (TransitionsCurrentMask != TransitionsMask)
	{
		if (TransitionsTask.IsValid() && Render->MeshThreadPool->RetractQueuedWork(TransitionsTask.Get()))
		{
			TransitionsTask.Reset();
		}
		UpdateTransitions();
	}

	if (Mesh)
	{
		for (int Index = 0; Index < SECTIONS_PER_CHUNK; Index++)
		{
			if (ChunksCurrentMask[Index] != TransitionsDisplayedMask)
			{
				FVoxelProcMeshSection Section;
				IntermediateChunks[Index].InitSectionBuffers(Section.ProcVertexBuffer, Section.ProcIndexBuffer, TransitionsDisplayedMask);
				ChunksCurrentMask[Index] = TransitionsDisplayedMask;
				Section.SectionLocalBox = FBox(-FVector::OneVector, (TotalSize() + 2) * FVector::OneVector);
				Section.bEnableCollision = ComputeCollisions();

				Mesh->SetProcMeshSection(Index, Section);
			}
		}
	}
}

UVoxelProceduralMeshComponent* FVoxelRenderChunk::GetMesh() const
{
	return Mesh;
}

FIntBox FVoxelRenderChunk::GetBounds() const
{
	check(Bounds == FIntBox(Position, Position + FIntVector(TotalSize() + 1, TotalSize() + 1, TotalSize() + 1)));
	return Bounds;
}

int FVoxelRenderChunk::TotalSize() const
{
	return RENDER_CHUNK_SIZE << LOD;
}

int FVoxelRenderChunk::SectionSize() const
{
	return CHUNK_SIZE << LOD;
}

bool FVoxelRenderChunk::ComputeCollisions() const
{
	return LOD <= Render->World->GetMaxCollisionsLOD();
}

const TArray<TSharedRef<FVoxelChunkToDelete>>& FVoxelRenderChunk::GetPreviousChunks() const
{
	return PreviousChunks;
}

void FVoxelRenderChunk::UpdateSection(int SectionIndex)
{
	check(!Tasks[SectionIndex].IsValid());

	const FIntVector& ChunkPosition = SectionsBounds[SectionIndex].Min;
	auto NewTask = MakeShared<FAsyncPolygonizerWork>(
		LOD,
		Render->World->GetData(),
		ChunkPosition,
		ChunkPosition - Position,
		Render->World
	);

	Render->MeshThreadPool->AddQueuedWork(&NewTask.Get());
	Tasks[SectionIndex] = NewTask;
	SectionNeedsUpdate[SectionIndex] = false;

	UpdateTransitions();
}

///////////////////////////////////////////////////////////////////////////////

FVoxelChunkToDelete::FVoxelChunkToDelete(const FVoxelRenderChunk& OldChunk)
	: RefCount(0)
	, Mesh(OldChunk.GetMesh())
	, Render(OldChunk.Render)
	, OldChunks(OldChunk.GetPreviousChunks())
	, bCollisions(OldChunk.ComputeCollisions())
{

}

void FVoxelChunkToDelete::AddRef()
{
	RefCount++;
}

void FVoxelChunkToDelete::RemoveRef()
{
	RefCount--;
	check(RefCount >= 0);
	if (RefCount == 0)
	{
		if (Mesh)
		{
			Render->RemoveMesh(Mesh, bCollisions);
		}
		for (auto& OldChunk : OldChunks)
		{
			OldChunk->RemoveRef();
		}
		OldChunks.Reset();
	}
}

int FVoxelChunkToDelete::GetRefCount() const
{
	return RefCount;
}

///////////////////////////////////////////////////////////////////////////////

FLODVoxelRender::FLODVoxelRender(AVoxelWorld* World, AActor* ChunksOwner)
	: FCollisionVoxelRender(World, ChunksOwner)
	, TimeSinceUpdate(0)
	, MeshThreadPool(new FVoxelQueuedThreadPool())
	, OctreeBuilderThreadPool(FQueuedThreadPool::Allocate())
{
	MeshThreadPool->Create(World->GetMeshThreadCount(), 1024 * 1024);
	OctreeBuilderThreadPool->Create(1, 1024 * 1024);
}

FLODVoxelRender::~FLODVoxelRender()
{
	// First delete pool
	MeshThreadPool->Destroy();
	delete MeshThreadPool;

	// Then delete chunks (and their tasks referenced in the pool)
	ChunksArray.Reset();
	Chunks.Reset();
	TasksToDelete.Reset();

	if (OctreeBuilder.IsValid() && !OctreeBuilder->Cancel())
	{
		OctreeBuilder->EnsureCompletion();
	}
	OctreeBuilderThreadPool->Destroy();
	delete OctreeBuilderThreadPool;
}

void FLODVoxelRender::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_Tick);
	
	FCollisionVoxelRender::Tick(DeltaTime);

	{
		for (auto& Chunk : MeshToRemove)
		{
			Chunk.Time += DeltaTime;
			if (Chunk.Time > 2 * World->GetChunksFadeDuration())
			{
				for (int Index = 0; Index < SECTIONS_PER_CHUNK + 1 /* Transitions */; Index++)
				{
					Chunk.Mesh->SetProcMeshSection(Index, FVoxelProcMeshSection());
				}
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
		MeshToRemove.RemoveAll([&](const FVoxelChunkToRemove& Chunk) { return Chunk.Time > 2 * World->GetChunksFadeDuration(); });
	}

	{
		for (auto& Task : TasksToDelete)
		{
			if (Task->IsDone())
			{
				Task.Reset();
			}
		}
		TasksToDelete.RemoveAll([&](TSharedPtr<FVoxelAsyncWork>& Ptr) { return !Ptr.IsValid(); });
	}

	for (auto& Chunk : ChunksArray)
	{
		Chunk->Tick();
	}

	if (OctreeBuilder.IsValid() && OctreeBuilder->IsDone())
	{
		UpdateLOD();
		OctreeBuilder.Reset();
	}

	{
		TimeSinceUpdate += DeltaTime;
		if (TimeSinceUpdate > 1 / World->GetLODUpdateRate() && !OctreeBuilder.IsValid())
		{
			TimeSinceUpdate = 0;

			TArray<FIntBox> CameraBounds;
			{
				Invokers.RemoveAll([](auto Ptr) { return !Ptr.IsValid(); });
				for (const auto& Invoker : Invokers)
				{
					check(Invoker.IsValid());
					if (Invoker->UseForRender())
					{
						CameraBounds.Add(Invoker->GetCameraBounds(World));
					}
				}
			}
			OctreeBuilder = MakeShared<FAsyncTask<FAsyncOctreeBuilderTask>>(CameraBounds, World->GetLOD() - CHUNK_MULTIPLIER_EXPONENT, World->GetLODLimit(), Octree);
			OctreeBuilder->StartBackgroundTask(OctreeBuilderThreadPool);
		}
	}
}

void FLODVoxelRender::AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker)
{
	FCollisionVoxelRender::AddInvoker(Invoker);

	Invokers.Add(Invoker);
}

void FLODVoxelRender::UpdateBoxInternal(const FIntBox& Box)
{
	FCollisionVoxelRender::UpdateBoxInternal(Box);

	if (Octree.IsValid())
	{
		TArray<FVoxelChunkOctree*> Leaves;
		Octree->GetLeavesOverlappingBox(Box, Leaves);
		for (auto& Chunk : Leaves)
		{
			check(Chunks.Contains(Chunk->GetBounds()));
			Chunks[Chunk->GetBounds()]->UpdateChunk(Box);
		}
	}
	else
	{
		//UE_LOG(LogVoxel, Error, TEXT("UpdateBox called too soon"));
	}
}

uint8 FLODVoxelRender::GetLODAtPosition(const FIntVector& Position) const
{
	return Octree->GetLeaf(Position)->LOD;
}

void FLODVoxelRender::RemoveMesh(UVoxelProceduralMeshComponent* Mesh, bool bCollisions)
{
	FVoxelChunkToRemove Chunk;
	Chunk.Mesh = Mesh;
	Chunk.Time = 0;
	Chunk.bCollisions = bCollisions;
	
	auto Mat = CastChecked<UMaterialInstanceDynamic>(Mesh->GetMaterial(0));
	Mat->SetScalarParameterValue(FName(TEXT("EndTime")), UGameplayStatics::GetTimeSeconds(World->GetWorld()) + World->GetChunksFadeDuration() * 2);

	MeshToRemove.Add(Chunk);
}

UVoxelProceduralMeshComponent* FLODVoxelRender::GetNewMesh(const FIntVector& Position, bool bCollisions, bool bInitialFade, uint8 LOD)
{
	UVoxelProceduralMeshComponent* NewMesh;
	if ((bCollisions ? InactiveMeshesCollisions : InactiveMeshesNoCollisions).Num() > 0)
	{
		// Create only if needed
		NewMesh = (bCollisions ? InactiveMeshesCollisions : InactiveMeshesNoCollisions).Pop();
	}
	else
	{
		NewMesh = NewObject<UVoxelProceduralMeshComponent>(ChunksOwner, NAME_None, RF_Transient);
		NewMesh->bUseAsyncCooking = true;
		NewMesh->SetupAttachment(ChunksOwner->GetRootComponent(), NAME_None);
		NewMesh->RegisterComponent();
		NewMesh->SetWorldScale3D(FVector::OneVector * World->GetVoxelSize());
		NewMesh->SetCollisionEnabled(bCollisions ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		if (bCollisions)
		{
			NewMesh->BodyInstance = World->GetCollisionPresets();
		}
		NewMesh->bCastShadowAsTwoSided = true;

		UMaterialInstanceDynamic* NewMat = UMaterialInstanceDynamic::Create(World->GetVoxelMaterial(), World);
		NewMat->CopyInterpParameters(World->GetVoxelMaterialDynamicInstance());
		NewMat->SetScalarParameterValue(FName(TEXT("EndTime")), 0); // Needed for first init, as 1e10 is too big
		NewMat->SetScalarParameterValue(FName(TEXT("FadeDuration")), World->GetChunksFadeDuration());

		for (int Index = 0; Index < SECTIONS_PER_CHUNK + 1 /* Transitions */; Index++)
		{
			NewMesh->SetMaterial(Index, NewMat);
		}
	}

	auto Mat = CastChecked<UMaterialInstanceDynamic>(NewMesh->GetMaterial(0));
	Mat->SetScalarParameterValue(FName(TEXT("StartTime")), bInitialFade ? UGameplayStatics::GetTimeSeconds(World->GetWorld()) : -World->GetChunksFadeDuration());
	Mat->SetScalarParameterValue(FName(TEXT("EndTime")), 1e10);
	Mat->SetScalarParameterValue(FName(TEXT("LOD")), LOD);

	NewMesh->SetWorldLocation(World->LocalToGlobal(Position) + ChunksOwner->GetActorLocation() - World->GetActorLocation());

	return NewMesh;
}


void FLODVoxelRender::AddTaskToDelete(const TSharedPtr<FVoxelAsyncWork>& NewTaskToDelete)
{
	TasksToDelete.Add(NewTaskToDelete);
}

void FLODVoxelRender::UpdateLOD()
{
	SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_UpdateLOD);

	check(OctreeBuilder.IsUnique() && OctreeBuilder->IsDone());

	TSharedPtr<FVoxelChunkOctree>& OldOctree = OctreeBuilder->GetTask().OldOctree;
	TSharedPtr<FVoxelChunkOctree>& NewOctree = OctreeBuilder->GetTask().NewOctree;
	Octree = NewOctree;

	TSet<FIntBox>& ChunksToDelete = OctreeBuilder->GetTask().ChunksToDelete;
	TSet<FIntBox>& ChunksToCreate = OctreeBuilder->GetTask().ChunksToCreate;

	TMap<FIntBox, uint8>& TransitionsMasks = OctreeBuilder->GetTask().TransitionsMasks;

	TMap<FIntBox, TSharedRef<FVoxelChunkToDelete>> ChunksToDeleteMap;

	{
		SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_UpdateLOD_ChunksToDelete);

		for (auto& Box : ChunksToDelete)
		{
			TSharedPtr<FVoxelRenderChunk> Chunk;
			ensure(Chunks.RemoveAndCopyValue(Box, Chunk));

			Chunk->EndTasks();
			if (Chunk->GetMesh() || Chunk->GetPreviousChunks().Num() > 0)
			{
				ChunksToDeleteMap.Add(Box, MakeShareable(new FVoxelChunkToDelete(*Chunk)));
			}
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_UpdateLOD_ChunksToCreate);

		for (auto& NewChunkBounds : ChunksToCreate)
		{
			TSharedRef<FVoxelRenderChunk> NewChunk = MakeShared<FVoxelRenderChunk>(this, NewChunkBounds.Min, FMath::RoundToInt(FMath::Log2(NewChunkBounds.Size().X / RENDER_CHUNK_SIZE)), NewChunkBounds);

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
				NewChunk->UpdateChunk(FIntBox::Infinite());
			}

			//check(!Chunks.Contains(NewChunkBounds));
			Chunks.Add(NewChunkBounds, NewChunk);
		}
	}

	// Rebuild chunks array
	Chunks.GenerateValueArray(ChunksArray);

	{
		SCOPE_CYCLE_COUNTER(STAT_LODVoxelRender_UpdateLOD_UpdateTransitions);

		for (auto& Chunk : ChunksArray)
		{
			Chunk->UpdateTransitions(TransitionsMasks[Chunk->Bounds]);
		}
	}
}