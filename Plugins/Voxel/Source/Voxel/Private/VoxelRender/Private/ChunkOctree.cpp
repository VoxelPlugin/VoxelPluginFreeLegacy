#include "VoxelPrivatePCH.h"
#include "ChunkOctree.h"
#include "VoxelChunkComponent.h"
#include "Engine/World.h"
#include "VoxelInvokerComponent.h"
#include "Math/NumericLimits.h"
#include "VoxelRender.h"

FChunkOctree::FChunkOctree(FVoxelRender* Render, FIntVector Position, uint8 Depth, uint64 Id)
	: FOctree(Position, Depth, Id)
	, Render(Render)
	, bHasChunk(false)
	, VoxelChunk(nullptr)
{
	check(Render);
};

void FChunkOctree::Delete()
{
	if (bHasChunk)
	{
		Unload();
	}
	if (bHasChilds)
	{
		DeleteChilds();
	}
}

void FChunkOctree::UpdateLOD(std::forward_list<TWeakObjectPtr<UVoxelInvokerComponent>> Invokers)
{
	check(bHasChunk == (VoxelChunk != nullptr));
	check(bHasChilds == (Childs.Num() == 8));
	check(!(bHasChilds && bHasChunk));

	if (Depth == 0)
	{
		// Always create
		if (!bHasChunk)
		{
			Load();
		}
		return;
	}

	const FVector ChunkWorldPosition = Render->GetGlobalPosition(Position);
	const float ChunkDiagonal = Render->World->GetVoxelSize() * Size() / 2 * 1.73205080757 /* sqrt(3) */;

	float MinDistance = MAX_flt;
	for (auto Invoker : Invokers)
	{
		if (Invoker.IsValid())
		{
			const float Distance = FMath::Max(0.f, (ChunkWorldPosition - Invoker->GetOwner()->GetActorLocation()).GetAbsMax() - Invoker->DistanceOffset - ChunkDiagonal);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
			}
		}
	}

	MinDistance /= Render->World->GetVoxelSize();

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
				Childs[i]->UpdateLOD(Invokers);
			}
		}
		else if (!bHasChunk)
		{
			// Not created, create
			Load();
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
				Childs[i]->UpdateLOD(Invokers);
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
			Load();
		}
	}
	check(bHasChilds ^ bHasChunk);
}

TWeakPtr<FChunkOctree> FChunkOctree::GetLeaf(FIntVector PointPosition)
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
		return GetChild(PointPosition)->GetLeaf(PointPosition);
	}
}

UVoxelChunkComponent* FChunkOctree::GetVoxelChunk() const
{
	return VoxelChunk;
}

TSharedPtr<FChunkOctree> FChunkOctree::GetChild(FIntVector PointPosition)
{
	check(bHasChilds);
	check(IsInOctree(PointPosition.X, PointPosition.Y, PointPosition.Z));

	// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
	TSharedPtr<FChunkOctree> Child = Childs[(PointPosition.X >= Position.X ? 1 : 0) + (PointPosition.Y >= Position.Y ? 2 : 0) + (PointPosition.Z >= Position.Z ? 4 : 0)];

	check(Child.IsValid());
	return Child;
}

void FChunkOctree::Load()
{
	check(!VoxelChunk);
	check(!bHasChunk);
	check(!bHasChilds);

	VoxelChunk = Render->GetInactiveChunk();
	VoxelChunk->Init(AsShared());
	Render->UpdateChunk(AsShared(), true);
	bHasChunk = true;
}

void FChunkOctree::Unload()
{
	check(VoxelChunk);
	check(bHasChunk);
	check(!bHasChilds);

	VoxelChunk->Unload();

	VoxelChunk = nullptr;
	bHasChunk = false;
}

void FChunkOctree::CreateChilds()
{
	check(!bHasChilds);
	check(!bHasChunk);
	check(Depth != 0);

	int d = Size() / 4;
	uint64 Pow = IntPow9(Depth - 1);

	Childs.Add(TSharedPtr<FChunkOctree>(new FChunkOctree(Render, Position + FIntVector(-d, -d, -d), Depth - 1, Id + 1 * Pow)));
	Childs.Add(TSharedPtr<FChunkOctree>(new FChunkOctree(Render, Position + FIntVector(+d, -d, -d), Depth - 1, Id + 2 * Pow)));
	Childs.Add(TSharedPtr<FChunkOctree>(new FChunkOctree(Render, Position + FIntVector(-d, +d, -d), Depth - 1, Id + 3 * Pow)));
	Childs.Add(TSharedPtr<FChunkOctree>(new FChunkOctree(Render, Position + FIntVector(+d, +d, -d), Depth - 1, Id + 4 * Pow)));
	Childs.Add(TSharedPtr<FChunkOctree>(new FChunkOctree(Render, Position + FIntVector(-d, -d, +d), Depth - 1, Id + 5 * Pow)));
	Childs.Add(TSharedPtr<FChunkOctree>(new FChunkOctree(Render, Position + FIntVector(+d, -d, +d), Depth - 1, Id + 6 * Pow)));
	Childs.Add(TSharedPtr<FChunkOctree>(new FChunkOctree(Render, Position + FIntVector(-d, +d, +d), Depth - 1, Id + 7 * Pow)));
	Childs.Add(TSharedPtr<FChunkOctree>(new FChunkOctree(Render, Position + FIntVector(+d, +d, +d), Depth - 1, Id + 8 * Pow)));

	bHasChilds = true;
}

void FChunkOctree::DeleteChilds()
{
	check(!bHasChunk);
	check(bHasChilds);
	check(Childs.Num() == 8);

	for (TSharedPtr<FChunkOctree> Child : Childs)
	{
		Child->Delete();
		Child.Reset();
	}
	Childs.Reset();
	bHasChilds = false;
}

TWeakPtr<FChunkOctree> FChunkOctree::GetAdjacentChunk(TransitionDirection Direction)
{
	const int S = Size();
	TArray<FIntVector> L = {
		FIntVector(-S, 0, 0),
		FIntVector(+S, 0, 0),
		FIntVector(0, -S, 0),
		FIntVector(0, +S, 0),
		FIntVector(0, 0, -S),
		FIntVector(0, 0, +S)
	};

	FIntVector P = Position + L[Direction];

	if (Render->Data->IsInWorld(P.X, P.Y, P.Z))
	{
		return Render->GetChunkOctreeAt(P);
	}
	else
	{
		return TWeakPtr<FChunkOctree>(nullptr);
	}
}

void FChunkOctree::GetLeafsOverlappingBox(FVoxelBox Box, std::forward_list<TWeakPtr<FChunkOctree>>& Octrees)
{
	FVoxelBox OctreeBox(GetMinimalCornerPosition(), GetMaximalCornerPosition());

	if (OctreeBox.Intersect(Box))
	{
		if (IsLeaf())
		{
			Octrees.push_front(AsShared());
		}
		else
		{
			for (auto Child : Childs)
			{
				Child->GetLeafsOverlappingBox(Box, Octrees);
			}
		}
	}
}
