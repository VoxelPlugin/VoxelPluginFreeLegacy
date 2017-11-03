// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelRender.h"
#include "VoxelChunkComponent.h"
#include "ChunkOctree.h"

DECLARE_CYCLE_STAT(TEXT("VoxelRender ~ ApplyUpdates"), STAT_ApplyUpdates, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelRender ~ UpdateLOD"), STAT_UpdateLOD, STATGROUP_Voxel);

FVoxelRender::FVoxelRender(AVoxelWorld* World, AActor* ChunksParent, FVoxelData* Data, uint32 MeshThreadCount, uint32 FoliageThreadCount)
	: World(World)
	, ChunksParent(ChunksParent)
	, Data(Data)
	, MeshThreadPool(FQueuedThreadPool::Allocate())
	, FoliageThreadPool(FQueuedThreadPool::Allocate())
	, TimeSinceFoliageUpdate(0)
	, TimeSinceLODUpdate(0)
{
	// Add existing chunks
	for (auto Component : ChunksParent->GetComponentsByClass(UVoxelChunkComponent::StaticClass()))
	{
		UVoxelChunkComponent* ChunkComponent = Cast<UVoxelChunkComponent>(Component);
		if (ChunkComponent)
		{
			ChunkComponent->SetProcMeshSection(0, FProcMeshSection());
			ChunkComponent->SetVoxelMaterial(World->GetVoxelMaterial());
			InactiveChunks.push_front(ChunkComponent);
		}
	}
	// Delete existing grass components
	for (auto Component : ChunksParent->GetComponentsByClass(UHierarchicalInstancedStaticMeshComponent::StaticClass()))
	{
		Component->DestroyComponent();
	}

	MeshThreadPool->Create(MeshThreadCount, 64 * 1024);
	FoliageThreadPool->Create(FoliageThreadCount, 32 * 1024);

	MainOctree = MakeShareable(new FChunkOctree(this, FIntVector::ZeroValue, Data->Depth, FOctree::GetTopIdFromDepth(Data->Depth)));
}

FVoxelRender::~FVoxelRender()
{
	check(ActiveChunks.Num() == 0);
	check(InactiveChunks.empty());
}

void FVoxelRender::Tick(float DeltaTime)
{
	check(ChunksToCheckForTransitionChange.Num() == 0);

	TimeSinceFoliageUpdate += DeltaTime;
	TimeSinceLODUpdate += DeltaTime;

	if (TimeSinceLODUpdate > 1 / World->GetLODUpdateFPS())
	{
		check(ChunksToCheckForTransitionChange.Num() == 0);

		UpdateLOD();

		// See Init and Unload functions of AVoxelChunk
		for (auto Chunk : ChunksToCheckForTransitionChange)
		{
			Chunk->CheckTransitions();
		}
		ChunksToCheckForTransitionChange.Empty();

		TimeSinceLODUpdate = 0;
	}

	ApplyUpdates();

	if (TimeSinceFoliageUpdate > 1 / World->GetFoliageFPS())
	{
		for (auto Chunk : FoliageUpdateNeeded)
		{
			Chunk->UpdateFoliage();
		}
		FoliageUpdateNeeded.Empty();
		TimeSinceFoliageUpdate = 0;
	}

	// Apply new meshes and new foliages
	{
		FScopeLock Lock(&ChunksToApplyNewMeshLock);
		for (auto Chunk : ChunksToApplyNewMesh)
		{
			Chunk->ApplyNewMesh();
		}
		ChunksToApplyNewMesh.Empty();
	}
	{
		FScopeLock Lock(&ChunksToApplyNewFoliageLock);
		for (auto Chunk : ChunksToApplyNewFoliage)
		{
			Chunk->ApplyNewFoliage();
		}
		ChunksToApplyNewFoliage.Empty();
	}

	// Chunks to delete
	for (FChunkToDelete& ChunkToDelete : ChunksToDelete)
	{
		ChunkToDelete.TimeLeft -= DeltaTime;
		if (ChunkToDelete.TimeLeft < 0)
		{
			auto Chunk = ChunkToDelete.Chunk;
			check(!FoliageUpdateNeeded.Contains(Chunk));
			check(!ChunksToApplyNewMesh.Contains(Chunk));
			check(!ChunksToApplyNewFoliage.Contains(Chunk));
			Chunk->Delete();
			ActiveChunks.Remove(Chunk);
			InactiveChunks.push_front(Chunk);
		}
	}
	ChunksToDelete.remove_if([](FChunkToDelete ChunkToDelete) { return ChunkToDelete.TimeLeft < 0; });
}

void FVoxelRender::AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker)
{
	VoxelInvokerComponents.push_front(Invoker);
}

UVoxelChunkComponent* FVoxelRender::GetInactiveChunk()
{
	UVoxelChunkComponent* Chunk;
	if (InactiveChunks.empty())
	{
		Chunk = NewObject<UVoxelChunkComponent>(ChunksParent, NAME_None);
		Chunk->SetupAttachment(ChunksParent->GetRootComponent(), NAME_None);
		Chunk->RegisterComponent();
		Chunk->SetVoxelMaterial(World->GetVoxelMaterial());
	}
	else
	{
		Chunk = InactiveChunks.front();
		InactiveChunks.pop_front();
	}
	ActiveChunks.Add(Chunk);

	check(Chunk->IsValidLowLevel());
	return Chunk;
}

void FVoxelRender::UpdateChunk(TWeakPtr<FChunkOctree> Chunk, bool bAsync)
{
	if (Chunk.IsValid())
	{
		ChunksToUpdate.Add(Chunk);
		if (!bAsync)
		{
			IdsOfChunksToUpdateSynchronously.Add(Chunk.Pin().Get()->Id);
		}
	}
}

void FVoxelRender::UpdateChunksAtPosition(FIntVector Position, bool bAsync)
{
	check(Data->IsInWorld(Position.X, Position.Y, Position.Z));

	int X = Position.X + Data->Size() / 2;
	int Y = Position.Y + Data->Size() / 2;
	int Z = Position.Z + Data->Size() / 2;

	bool bXIsAtBorder = (X % 16 == 0) && (X != 0);
	bool bYIsAtBorder = (Y % 16 == 0) && (Y != 0);
	bool bZIsAtBorder = (Z % 16 == 0) && (Z != 0);

	UpdateChunk(MainOctree->GetLeaf(Position), bAsync);

	if (bXIsAtBorder)
	{
		UpdateChunk(MainOctree->GetLeaf(Position - FIntVector(8, 0, 0)), bAsync);
	}
	if (bYIsAtBorder)
	{
		UpdateChunk(MainOctree->GetLeaf(Position - FIntVector(0, 8, 0)), bAsync);
	}
	if (bXIsAtBorder && bYIsAtBorder)
	{
		UpdateChunk(MainOctree->GetLeaf(Position - FIntVector(8, 8, 0)), bAsync);
	}
	if (bZIsAtBorder)
	{
		UpdateChunk(MainOctree->GetLeaf(Position - FIntVector(0, 0, 8)), bAsync);
	}
	if (bXIsAtBorder && bZIsAtBorder)
	{
		UpdateChunk(MainOctree->GetLeaf(Position - FIntVector(8, 0, 8)), bAsync);
	}
	if (bYIsAtBorder && bZIsAtBorder)
	{
		UpdateChunk(MainOctree->GetLeaf(Position - FIntVector(0, 8, 8)), bAsync);
	}
	if (bXIsAtBorder && bYIsAtBorder && bZIsAtBorder)
	{
		UpdateChunk(MainOctree->GetLeaf(Position - FIntVector(8, 8, 8)), bAsync);
	}
}

void FVoxelRender::UpdateChunksOverlappingBox(FVoxelBox Box, bool bAsync)
{
	std::forward_list<TWeakPtr<FChunkOctree>> OverlappingLeafs;
	MainOctree->GetLeafsOverlappingBox(Box, OverlappingLeafs);

	for (auto Chunk : OverlappingLeafs)
	{
		UpdateChunk(Chunk, bAsync);
	}
}

void FVoxelRender::ApplyUpdates()
{
	SCOPE_CYCLE_COUNTER(STAT_ApplyUpdates);

	std::forward_list<TWeakPtr<FChunkOctree>> Failed;

	for (auto& Chunk : ChunksToUpdate)
	{
		TSharedPtr<FChunkOctree> LockedChunk(Chunk.Pin());

		if (LockedChunk.IsValid() && LockedChunk->GetVoxelChunk())
		{
			bool bAsync = !IdsOfChunksToUpdateSynchronously.Contains(LockedChunk->Id);
			bool bSuccess = LockedChunk->GetVoxelChunk()->Update(bAsync);

			/*if (!bSuccess)
			{
				UE_LOG(VoxelLog, Warning, TEXT("Chunk already updating"));
			}*/
		}
	}
	ChunksToUpdate.Reset();
	IdsOfChunksToUpdateSynchronously.Reset();
}

void FVoxelRender::UpdateAll(bool bAsync)
{
	for (auto Chunk : ActiveChunks)
	{
		Chunk->Update(bAsync);
	}
}

void FVoxelRender::UpdateLOD()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateLOD);

	// Clean
	std::forward_list<TWeakObjectPtr<UVoxelInvokerComponent>> Temp;
	for (auto Invoker : VoxelInvokerComponents)
	{
		if (Invoker.IsValid())
		{
			Temp.push_front(Invoker);
		}
	}
	VoxelInvokerComponents = Temp;

	MainOctree->UpdateLOD(VoxelInvokerComponents);
}

void FVoxelRender::AddFoliageUpdate(UVoxelChunkComponent* Chunk)
{
#if DO_CHECK
	for (auto ChunkToDelete : ChunksToDelete)
	{
		//check(ChunkToDelete.Chunk != Chunk);
	}
#endif

	FoliageUpdateNeeded.Add(Chunk);
}

void FVoxelRender::AddTransitionCheck(UVoxelChunkComponent* Chunk)
{
	ChunksToCheckForTransitionChange.Add(Chunk);
}

void FVoxelRender::ScheduleDeletion(UVoxelChunkComponent* Chunk)
{
	// Cancel any pending update
	RemoveFromQueues(Chunk);

	// Schedule deletion
	ChunksToDelete.push_front(FChunkToDelete(Chunk, World->GetDeletionDelay()));
}

void FVoxelRender::ChunkHasBeenDestroyed(UVoxelChunkComponent* Chunk)
{
	RemoveFromQueues(Chunk);

	ChunksToDelete.remove_if([Chunk](FChunkToDelete ChunkToDelete) { return ChunkToDelete.Chunk == Chunk; });

	InactiveChunks.remove(Chunk);
	ActiveChunks.Remove(Chunk);
}

void FVoxelRender::AddApplyNewMesh(UVoxelChunkComponent* Chunk)
{
	FScopeLock Lock(&ChunksToApplyNewMeshLock);
	ChunksToApplyNewMesh.Add(Chunk);
}

void FVoxelRender::AddApplyNewFoliage(UVoxelChunkComponent* Chunk)
{
	FScopeLock Lock(&ChunksToApplyNewFoliageLock);
	ChunksToApplyNewFoliage.Add(Chunk);
}

void FVoxelRender::RemoveFromQueues(UVoxelChunkComponent* Chunk)
{
	FoliageUpdateNeeded.Remove(Chunk);

	{
		FScopeLock Lock(&ChunksToApplyNewMeshLock);
		ChunksToApplyNewMesh.Remove(Chunk);
	}
	{
		FScopeLock Lock(&ChunksToApplyNewFoliageLock);
		ChunksToApplyNewFoliage.Remove(Chunk);
	}
}

TWeakPtr<FChunkOctree> FVoxelRender::GetChunkOctreeAt(FIntVector Position) const
{
	check(Data->IsInWorld(Position.X, Position.Y, Position.Z));
	return MainOctree->GetLeaf(Position);
}

int FVoxelRender::GetDepthAt(FIntVector Position) const
{
	return GetChunkOctreeAt(Position).Pin()->Depth;
}

void FVoxelRender::Destroy()
{
	MeshThreadPool->Destroy();
	FoliageThreadPool->Destroy();

	for (auto Chunk : ActiveChunks)
	{
		if (!Chunk->IsPendingKill())
		{
			Chunk->Delete();
		}
		else
		{
			Chunk->ResetRender();
		}
	}

	ActiveChunks.Empty();
	InactiveChunks.resize(0);
}

FVector FVoxelRender::GetGlobalPosition(FIntVector LocalPosition)
{
	return World->LocalToGlobal(LocalPosition) + ChunksParent->GetActorLocation() - World->GetActorLocation();
}
