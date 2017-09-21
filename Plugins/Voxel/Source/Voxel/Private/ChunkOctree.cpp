#include "VoxelPrivatePCH.h"
#include "ChunkOctree.h"
#include "VoxelChunk.h"
#include "Engine/World.h"
#include "VoxelInvokerComponent.h"
#include "Math/NumericLimits.h"

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

void ChunkOctree::UpdateLOD(AVoxelWorld* World, std::forward_list<TWeakObjectPtr<UVoxelInvokerComponent>> Invokers)
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

	const FVector ChunkWorldPosition = World->GetTransform().TransformPosition((FVector)Position);
	const float ChunkDiagonal = World->GetActorScale3D().X * Width() / 2 * 1.73205080757 /* sqrt(3) */;

	float MinDistance = MAX_flt;
	for (auto Invoker : Invokers)
	{
		if (Invoker != NULL)
		{
			const float Distance = FMath::Max(0.f, FVector::Distance(ChunkWorldPosition, Invoker->GetOwner()->GetActorLocation()) - Invoker->DistanceOffset - ChunkDiagonal);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
			}
		}
	}

	MinDistance /= World->GetActorScale3D().X;

	MinDistance = FMath::Max(1.f, MinDistance);

	const float MinLOD = FMath::Log2(MinDistance / 16);

	// Tolerance zone to avoid reload loop when on a border
	const float MaxDistance = MinDistance + (16 << (int)MinLOD) / 2;
	const float MaxLOD = FMath::Log2(MaxDistance / 16);

	if (MinLOD < Depth && Depth < MaxLOD)
	{
		// Depth OK
		if (bHasChilds)
		{
			// Update childs
			for (int i = 0; i < 8; i++)
			{
				check(Childs[i].IsValid());
				Childs[i]->UpdateLOD(World, Invokers);
			}
		}
		else if (!bHasChunk)
		{
			// Not created, create
			Load(World);
		}
	}
	else if (MaxLOD < Depth)
	{
		// Resolution too low
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
				Childs[i]->UpdateLOD(World, Invokers);
			}
		}
	}
	else // Depth < MinLOD
	{
		// Resolution too high
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

	VoxelChunk = World->GetChunkFromPool();
	VoxelChunk->Init(Position - FIntVector(1, 1, 1) * Width() / 2, Depth, World);
	World->QueueUpdate(AsShared(), true);
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
	uint64 Pow = IntPow9(Depth - 1);

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