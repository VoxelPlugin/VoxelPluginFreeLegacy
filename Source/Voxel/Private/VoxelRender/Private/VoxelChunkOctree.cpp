// Copyright 2018 Phyronnaz

#include "VoxelChunkOctree.h"

FVoxelChunkOctree::FVoxelChunkOctree(const TArray<FIntBox>& CameraBounds, uint8 LOD)
	: TVoxelOctree(LOD)
	, Root(this)
	, CameraBounds(CameraBounds)
{
	check(LOD > 0);

	CreateChilds();

	bContinueInit = true;
	while (bContinueInit)
	{
		bContinueInit = false;
		Init();
	}
}

FVoxelChunkOctree::FVoxelChunkOctree(FVoxelChunkOctree* Parent, uint8 ChildIndex)
	: TVoxelOctree(Parent, ChildIndex)
	, Root(Parent->Root)
{
	if (LOD > 0)
	{
		for (const FIntBox& Bound : Root->CameraBounds)
		{
			if (GetBounds().Intersect(Bound))
			{
				CreateChilds();
				break;
			}
		}
	}
}

void FVoxelChunkOctree::GetLeavesBounds(TSet<FIntBox>& InBounds) const
{
	if (IsLeaf())
	{
		InBounds.Add(GetBounds());
	}
	else
	{
		for (auto Child : GetChilds())
		{
			Child->GetLeavesBounds(InBounds);
		}
	}
}

void FVoxelChunkOctree::GetLeavesTransitionsMasks(TMap<FIntBox, uint8>& TransitionsMasks) const
{
	if (IsLeaf())
	{
		uint8 TransitionsMask = 0;
		for (auto Direction : { XMin, XMax, YMin, YMax, ZMin, ZMax })
		{
			FVoxelChunkOctree* AdjacentChunk = GetAdjacentChunk(Direction);
			if (AdjacentChunk && AdjacentChunk->LOD < LOD)
			{
				TransitionsMask |= Direction;
			}
		}
		TransitionsMasks.Add(GetBounds(), TransitionsMask);
	}
	else
	{
		for (auto Child : GetChilds())
		{
			Child->GetLeavesTransitionsMasks(TransitionsMasks);
		}
	}
}

FVoxelChunkOctree* FVoxelChunkOctree::GetAdjacentChunk(EVoxelDirection Direction) const
{
	FIntVector P;
	switch (Direction)
	{
	case XMin:
		P = Position - FIntVector(Size(), 0, 0);
		break;
	case XMax:
		P = Position + FIntVector(Size(), 0, 0);
		break;
	case YMin:
		P = Position - FIntVector(0, Size(), 0);
		break;
	case YMax:
		P = Position + FIntVector(0, Size(), 0);
		break;
	case ZMin:
		P = Position - FIntVector(0, 0, Size());
		break;
	case ZMax:
		P = Position + FIntVector(0, 0, Size());
		break;
	default:
		check(false);
		P = FIntVector::ZeroValue;
	}

	if (Root->IsInOctree(P))
	{
		return Root->GetLeaf(P);
	}
	else
	{
		return nullptr;
	}
}

void FVoxelChunkOctree::Init()
{
	if (IsLeaf())
	{
		for (auto Direction : { XMin, XMax, YMin, YMax, ZMin, ZMax })
		{
			FVoxelChunkOctree* AdjacentChunk = GetAdjacentChunk(Direction);
			while (AdjacentChunk && AdjacentChunk->LOD > LOD + 1)
			{
				AdjacentChunk->CreateChilds();
				AdjacentChunk = GetAdjacentChunk(Direction);
				Root->bContinueInit = true;
			}
			check(!GetAdjacentChunk(Direction) || GetAdjacentChunk(Direction)->LOD <= LOD + 1);
		}
	}
	else
	{
		for (auto Child : GetChilds())
		{
			Child->Init();
		}
	}
}
