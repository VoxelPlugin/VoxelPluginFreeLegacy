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

	ApplyUpdates();

	ApplyFoliageUpdates();
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

	for (auto& Chunk : ChunksToUpdate)
	{
		TSharedPtr<ChunkOctree> LockedChunk(Chunk.Pin());

		if (LockedChunk.IsValid() && LockedChunk->GetVoxelChunk())
		{
			bool bAsync = !IdsOfChunksToUpdateSynchronously.Contains(LockedChunk->Id);
			LockedChunk->GetVoxelChunk()->Update(bAsync);
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

void VoxelRender::ApplyFoliageUpdates()
{
	for (auto Chunk : FoliageUpdateNeeded)
	{
		Chunk->UpdateFoliage();
	}
	FoliageUpdateNeeded.Empty();
}

void VoxelRender::ApplyTransitionChecks()
{
	for (auto Chunk : ChunksToCheckForTransitionChange)
	{
		Chunk->CheckTransitions();
	}
	ChunksToCheckForTransitionChange.Empty();
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
