// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelRender.h"
#include "VoxelChunk.h"
#include "ChunkOctree.h"

DECLARE_CYCLE_STAT(TEXT("VoxelRender ~ ApplyUpdates"), STAT_ApplyUpdates, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelRender ~ UpdateLOD"), STAT_UpdateLOD, STATGROUP_Voxel);

VoxelRender::VoxelRender(AVoxelWorld* World, uint32 MeshThreadCount, uint32 FoliageThreadCount)
	: World(World)
	, MeshThreadPool(FQueuedThreadPool::Allocate())
	, FoliageThreadPool(FQueuedThreadPool::Allocate())
	, TimeSinceMeshUpdate(0)
	, TimeSinceFoliageUpdate(0)
{
	MeshThreadPool->Create(MeshThreadCount, 64 * 1024);
	FoliageThreadPool->Create(FoliageThreadCount, 32 * 1024);

	MainOctree = MakeShareable(new ChunkOctree(this, FIntVector::ZeroValue, World->Depth, Octree::GetTopIdFromDepth(World->Depth)));
}

void VoxelRender::Tick(float DeltaTime)
{
	TimeSinceMeshUpdate += DeltaTime;
	TimeSinceFoliageUpdate += DeltaTime;

	UpdateLOD();

	if (TimeSinceMeshUpdate > 1 / World->MeshFPS)
	{
		ApplyUpdates();
		TimeSinceMeshUpdate = 0;
	}

	if (TimeSinceFoliageUpdate > 1 / World->FoliageFPS)
	{
		ApplyFoliageUpdates();
		TimeSinceFoliageUpdate = 0;
	}

	ApplyNewMeshes();
	ApplyNewFoliages();
}

void VoxelRender::AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker)
{
	VoxelInvokerComponents.push_front(Invoker);
}

AVoxelChunk* VoxelRender::GetInactiveChunk()
{
	AVoxelChunk* Chunk;
	if (InactiveChunks.empty())
	{
		Chunk = World->GetWorld()->SpawnActor<AVoxelChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
		Chunk->AttachToActor(World, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
		Chunk->SetMaterial(World->VoxelMaterial);
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

void VoxelRender::SetChunkAsInactive(AVoxelChunk* Chunk)
{
	ActiveChunks.Remove(Chunk);
	InactiveChunks.push_front(Chunk);

	// Remove from queues
	ChunksToCheckForTransitionChange.Remove(Chunk);
	{
		FScopeLock Lock(&ChunksToApplyNewMeshLock);
		ChunksToApplyNewMesh.Remove(Chunk);
	}
	{
		FScopeLock Lock(&ChunksToApplyNewFoliageLock);
		ChunksToApplyNewFoliage.Remove(Chunk);
	}
}

void VoxelRender::UpdateChunk(TWeakPtr<ChunkOctree> Chunk, bool bAsync)
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

void VoxelRender::UpdateChunksAtPosition(FIntVector Position, bool bAsync)
{
	check(World->IsInWorld(Position));

	int X = Position.X + World->Size() / 2;
	int Y = Position.Y + World->Size() / 2;
	int Z = Position.Z + World->Size() / 2;

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

void VoxelRender::ApplyUpdates()
{
	SCOPE_CYCLE_COUNTER(STAT_ApplyUpdates);

	std::forward_list<TWeakPtr<ChunkOctree>> Failed;

	for (auto& Chunk : ChunksToUpdate)
	{
		TSharedPtr<ChunkOctree> LockedChunk(Chunk.Pin());

		if (LockedChunk.IsValid() && LockedChunk->GetVoxelChunk())
		{
			bool bAsync = !IdsOfChunksToUpdateSynchronously.Contains(LockedChunk->Id);
			bool bSuccess = LockedChunk->GetVoxelChunk()->Update(bAsync);
			if (!bSuccess)
			{
				// if not async always succeed
				Failed.push_front(Chunk);
			}
		}
		else
		{
			UE_LOG(VoxelLog, Warning, TEXT("Invalid chunk in queue"));
		}
	}
	ChunksToUpdate.Reset();
	IdsOfChunksToUpdateSynchronously.Reset();

	// See Init and Unload functions of AVoxelChunk
	ApplyTransitionChecks();

	// Add back chunks that were already updating
	for (auto Chunk : Failed)
	{
		ChunksToUpdate.Add(Chunk);
	}
}

void VoxelRender::UpdateAll(bool bAsync)
{
	for (auto Chunk : ActiveChunks)
	{
		Chunk->Update(bAsync);
	}
}

void VoxelRender::UpdateLOD()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateLOD);
	MainOctree->UpdateLOD(VoxelInvokerComponents);
}

void VoxelRender::AddFoliageUpdate(AVoxelChunk* Chunk)
{
	FoliageUpdateNeeded.Add(Chunk);
}

void VoxelRender::AddTransitionCheck(AVoxelChunk* Chunk)
{
	ChunksToCheckForTransitionChange.Add(Chunk);
}

void VoxelRender::AddApplyNewMesh(AVoxelChunk* Chunk)
{
	FScopeLock Lock(&ChunksToApplyNewMeshLock);
	ChunksToApplyNewMesh.Add(Chunk);
}

void VoxelRender::AddApplyNewFoliage(AVoxelChunk* Chunk)
{
	FScopeLock Lock(&ChunksToApplyNewFoliageLock);
	ChunksToApplyNewFoliage.Add(Chunk);
}

void VoxelRender::ApplyFoliageUpdates()
{
	std::forward_list<AVoxelChunk*> Failed;
	for (auto Chunk : FoliageUpdateNeeded)
	{
		bool bSuccess = Chunk->UpdateFoliage();
		if (!bSuccess)
		{
			Failed.push_front(Chunk);
		}
	}
	FoliageUpdateNeeded.Empty();

	for (auto Chunk : Failed)
	{
		FoliageUpdateNeeded.Add(Chunk);
	}
}

void VoxelRender::ApplyTransitionChecks()
{
	for (auto Chunk : ChunksToCheckForTransitionChange)
	{
		Chunk->CheckTransitions();
	}
	ChunksToCheckForTransitionChange.Empty();
}

void VoxelRender::ApplyNewMeshes()
{
	FScopeLock Lock(&ChunksToApplyNewMeshLock);
	for (auto Chunk : ChunksToApplyNewMesh)
	{
		Chunk->ApplyNewMesh();
	}
	ChunksToApplyNewMesh.Empty();
}

void VoxelRender::ApplyNewFoliages()
{
	FScopeLock Lock(&ChunksToApplyNewFoliageLock);
	for (auto Chunk : ChunksToApplyNewFoliage)
	{
		Chunk->ApplyNewFoliage();
	}
	ChunksToApplyNewFoliage.Empty();
}

TWeakPtr<ChunkOctree> VoxelRender::GetChunkOctreeAt(FIntVector Position) const
{
	check(World->IsInWorld(Position));
	return MainOctree->GetLeaf(Position);
}

int VoxelRender::GetDepthAt(FIntVector Position) const
{
	return GetChunkOctreeAt(Position).Pin()->Depth;
}
