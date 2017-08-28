#include "VoxelPrivatePCH.h"
#include "ChunkOctree.h"
#include "VoxelChunk.h"
#include "Engine/World.h"

void ChunkOctree::Delete()
{
	if (bHasChunk)
	{
		Unload();
	}
	if (bHasChilds)
	{
		DeleteChilds();
	}
	check(!bHasChunk && !bHasChilds);
}

void ChunkOctree::ImmediateDelete()
{
	if (bHasChunk)
	{
		VoxelChunk->Delete();
		VoxelChunk = nullptr;
		bHasChunk = false;
	}
	if (bHasChilds)
	{
		DeleteChilds();
	}
}

void ChunkOctree::UpdateCameraPosition(AVoxelWorld* World, FVector CameraPosition)
{
	check(World);
	check(bHasChunk == (VoxelChunk != nullptr));
	check(bHasChilds == (Childs.Num() == 8));
	check(!(bHasChilds && bHasChunk));

	if (Depth == 0)
	{
		// Always create
		if (!bHasChunk)
		{
			Load(World);
		}
		return;
	}

	const float DistanceToCamera = ((World->GetTransform().TransformPosition((FVector)Position) - CameraPosition).Size()) / World->GetActorScale3D().Size() - World->GetHighResolutionDistanceOffset();
	const float Quality = World->GetQuality();

	const float MinDistance = Width() * Quality;
	const float MaxDistance = Width() * Quality * 2;


	if (DistanceToCamera < 0)
	{
		// Always at highest resolution
		if (bHasChunk)
		{
			Unload();
		}
		if (!bHasChilds)
		{
			CreateChilds();
		}
		if (bHasChilds)
		{
			// Update childs
			for (int i = 0; i < 8; i++)
			{
				check(Childs[i].IsValid());
				Childs[i]->UpdateCameraPosition(World, CameraPosition);
			}
		}
		return;
	}


	if (MinDistance < DistanceToCamera && DistanceToCamera < MaxDistance)
	{
		// Depth OK
		if (bHasChilds)
		{
			// Update childs
			for (int i = 0; i < 8; i++)
			{
				check(Childs[i].IsValid());
				Childs[i]->UpdateCameraPosition(World, CameraPosition);
			}
		}
		else if (!bHasChunk)
		{
			// Not created, create
			Load(World);
		}
	}
	else if (DistanceToCamera < MinDistance)
	{
		// Depth too high
		if (bHasChunk)
		{
			Unload();
		}
		if (!bHasChilds)
		{
			CreateChilds();
		}
		if (bHasChilds)
		{
			// Update childs
			for (int i = 0; i < 8; i++)
			{
				check(Childs[i].IsValid());
				Childs[i]->UpdateCameraPosition(World, CameraPosition);
			}
		}
	}
	else // DistanceToCamera > MaxDistance
	{
		// Depth too low
		if (bHasChilds)
		{
			// Too far, delete childs
			DeleteChilds();
		}
		if (!bHasChunk)
		{
			// Not created, create
			Load(World);
		}
	}
	check(bHasChilds ^ bHasChunk);
}

void ChunkOctree::Update(bool bAsync)
{
	check(bHasChunk == (VoxelChunk != nullptr));
	check(bHasChilds == (Childs.Num() == 8));
	check(bHasChilds ^ bHasChunk);

	if (bHasChunk)
	{
		VoxelChunk->Update(bAsync);
	}
	else // bHasChilds
	{
		for (auto Child : Childs)
		{
			check(Child.IsValid());
			Child->Update(bAsync);
		}
	}
}

TWeakPtr<ChunkOctree> ChunkOctree::GetChunk(FIntVector PointPosition)
{
	check(bHasChunk == (VoxelChunk != nullptr));
	check(bHasChilds == (Childs.Num() == 8));
	check(bHasChilds ^ bHasChunk);

	if (bHasChunk)
	{
		return AsShared();
	}
	else // bHasChilds
	{
		return GetChild(PointPosition)->GetChunk(PointPosition);
	}
}

AVoxelChunk* ChunkOctree::GetVoxelChunk() const
{
	return VoxelChunk;
}

TSharedPtr<ChunkOctree> ChunkOctree::GetChild(FIntVector PointPosition)
{
	check(bHasChilds);
	check(IsInOctree(PointPosition));

	// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
	TSharedPtr<ChunkOctree> Child = Childs[(PointPosition.X >= Position.X ? 1 : 0) + (PointPosition.Y >= Position.Y ? 2 : 0) + (PointPosition.Z >= Position.Z ? 4 : 0)];

	check(Child.IsValid());
	return Child;
}

void ChunkOctree::Load(AVoxelWorld* World)
{
	check(VoxelChunk == nullptr);
	check(!bHasChunk);
	check(!bHasChilds);
	check(World);

	VoxelChunk = World->GetWorld()->SpawnActor<AVoxelChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
	VoxelChunk->Init(Position - FIntVector(1, 1, 1) * Width() / 2, Depth, World);
	World->QueueUpdate(AsShared());
	bHasChunk = true;
}

void ChunkOctree::Unload()
{
	check(VoxelChunk != nullptr);
	check(bHasChunk);
	check(!bHasChilds);

	VoxelChunk->Unload();

	VoxelChunk = nullptr;
	bHasChunk = false;
}

void ChunkOctree::CreateChilds()
{
	check(!bHasChilds);
	check(!bHasChunk);
	check(Depth != 0);

	int d = Width() / 4;
	uint32 Pow = IntPow9(Depth - 1);

	Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(-d, -d, -d), Depth - 1, Id + 1 * Pow)));
	Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(+d, -d, -d), Depth - 1, Id + 2 * Pow)));
	Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(-d, +d, -d), Depth - 1, Id + 3 * Pow)));
	Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(+d, +d, -d), Depth - 1, Id + 4 * Pow)));
	Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(-d, -d, +d), Depth - 1, Id + 5 * Pow)));
	Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(+d, -d, +d), Depth - 1, Id + 6 * Pow)));
	Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(-d, +d, +d), Depth - 1, Id + 7 * Pow)));
	Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(+d, +d, +d), Depth - 1, Id + 8 * Pow)));

	bHasChilds = true;
}

void ChunkOctree::DeleteChilds()
{
	check(!bHasChunk);
	check(bHasChilds);
	check(Childs.Num() == 8);

	for (TSharedPtr<ChunkOctree> Child : Childs)
	{
		Child->Delete();
		Child.Reset();
	}
	Childs.Reset();
	bHasChilds = false;
}