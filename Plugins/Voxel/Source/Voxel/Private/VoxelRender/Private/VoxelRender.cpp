// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelRender.h"
#include "VoxelChunk.h"
#include "ChunkOctree.h"

DECLARE_CYCLE_STAT(TEXT("VoxelRender ~ ApplyUpdates"), STAT_ApplyUpdates, STATGROUP_Voxel);

VoxelRender::VoxelRender(AVoxelWorld* World, uint32 MeshThreadCount, uint32 FoliageThreadCount)
	: World(World)
	, MeshThreadPool(FQueuedThreadPool::Allocate())
	, FoliageThreadPool(FQueuedThreadPool::Allocate())
{
	MeshThreadPool->Create(MeshThreadCount, 64 * 1024);
	FoliageThreadPool->Create(FoliageThreadCount, 32 * 1024);

	MainOctree = MakeShareable(new ChunkOctree(FIntVector::ZeroValue, World->GetDepth(), Octree::GetTopIdForDepth(World->GetDepth())));
}

AVoxelChunk* VoxelRender::GetInactiveChunk()
{
	AVoxelChunk* Chunk;
	if (InactiveChunks.empty())
	{
		Chunk = World->GetWorld()->SpawnActor<AVoxelChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
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
		QueuedChunks.Add(Chunk);
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

	UpdateChunk(MainOctree->GetChunk(Position), bAsync);

	if (bXIsAtBorder)
	{
		UpdateChunk(MainOctree->GetChunk(Position - FIntVector(8, 0, 0)), bAsync);
	}
	if (bYIsAtBorder)
	{
		UpdateChunk(MainOctree->GetChunk(Position - FIntVector(0, 8, 0)), bAsync);
	}
	if (bXIsAtBorder && bYIsAtBorder)
	{
		UpdateChunk(MainOctree->GetChunk(Position - FIntVector(8, 8, 0)), bAsync);
	}
	if (bZIsAtBorder)
	{
		UpdateChunk(MainOctree->GetChunk(Position - FIntVector(0, 0, 8)), bAsync);
	}
	if (bXIsAtBorder && bZIsAtBorder)
	{
		UpdateChunk(MainOctree->GetChunk(Position - FIntVector(8, 0, 8)), bAsync);
	}
	if (bYIsAtBorder && bZIsAtBorder)
	{
		UpdateChunk(MainOctree->GetChunk(Position - FIntVector(0, 8, 8)), bAsync);
	}
	if (bXIsAtBorder && bYIsAtBorder && bZIsAtBorder)
	{
		UpdateChunk(MainOctree->GetChunk(Position - FIntVector(8, 8, 8)), bAsync);
	}
}

void VoxelRender::ApplyUpdates()
{
	SCOPE_CYCLE_COUNTER(STAT_ApplyUpdates);

	for (auto& Chunk : QueuedChunks)
	{
		TSharedPtr<ChunkOctree> LockedChunk(Chunk.Pin());

		if (LockedChunk.IsValid())
		{
			bool bAsync = !IdsOfChunksToUpdateSynchronously.Contains(LockedChunk->Id);
			LockedChunk->Update(bAsync);
		}
		else
		{
			UE_LOG(VoxelLog, Warning, TEXT("Invalid chunk in queue"));
		}
	}
	QueuedChunks.Reset();
	IdsOfChunksToUpdateSynchronously.Reset();
}