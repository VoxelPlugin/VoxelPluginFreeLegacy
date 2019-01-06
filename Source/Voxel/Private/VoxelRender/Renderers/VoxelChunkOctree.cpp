// Copyright 2019 Phyronnaz

#include "VoxelChunkOctree.h"
#include "VoxelLogStatDefinitions.h"

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
	uint8 InLODLimit,
	uint8 InLODLowerLimit,
	int InMaxOctreeLeaves)
{
	check(!bIsActive);

	CameraPositions = InCameraPositions;
	Octree = InOctree;
	LODToMinDistance = InLODToMinDistance;
	VoxelSize = InVoxelSize;
	LODLimit = InLODLimit;
	LODLowerLimit = InLODLowerLimit;
	MaxOctreeLeaves = InMaxOctreeLeaves;

	bIsActive = true;
	StartTime = std::chrono::high_resolution_clock::now();
}

void FAsyncOctreeBuilderTask::Reset()
{
	check(bIsDone || !bIsActive);
	bIsActive = false;
	bIsDone = false;
}

template<typename T>
inline float GetDelta(const T& A, const T& B)
{
	return std::chrono::duration_cast<std::chrono::microseconds>(B - A).count() / 1000000.f;
}

void FAsyncOctreeBuilderTask::Log() const
{
	auto Now = std::chrono::high_resolution_clock::now();
	float TotalTime = GetDelta(StartTime, Now);
	float TimeInQueue = GetDelta(StartTime, WorkStartTime);
	float TimeWorking = GetDelta(WorkStartTime, WorkEndTime);
	float TimeWaitingForUpdate = GetDelta(WorkEndTime, Now);

	UE_LOG(LogVoxel, Log, TEXT(
		"Octree stats: Total time: %f; "
		"Time waiting in thread queue: %f; "
		"Time doing real work: %f; "
		"Time waiting for LOD Update Rate delay: %f; "
		"Thread used: %s; "
		"Number of leaves: %d"),
		TotalTime,
		TimeInQueue,
		TimeWorking,
		TimeWaitingForUpdate,
		bGameThreadUsed ? TEXT("Gamethread") : TEXT("Other thread"),
		NumberOfLeaves);
}

void FAsyncOctreeBuilderTask::DoWork()
{
	check(bIsActive);
	WorkStartTime = std::chrono::high_resolution_clock::now();

	bGameThreadUsed = IsInGameThread();

	ChunksToDelete.Reset();
	ChunksToCreate.Reset();
	NewOctree.Reset();
	OldOctree.Reset();
	TransitionsMasks.Reset();

	TArray<uint64> SquaredDistances;
	SquaredDistances.SetNum(MAX_WORLD_DEPTH);
	for (int Index = 1; Index < MAX_WORLD_DEPTH; Index++)
	{
		// +1: We want to divide LOD 2 if LOD _1_ min distance isn't met, not if LOD _2_ min distance isn't met
		int CurrentLOD = Index - 1;
		while (!LODToMinDistance.Contains(CurrentLOD))
		{
			check(CurrentLOD > 0);
			CurrentLOD--;
		}
		SquaredDistances[Index] = FMath::Square<uint64>(FMath::CeilToInt(LODToMinDistance[CurrentLOD] / VoxelSize));
	}
	FVoxelChunkOctreeSettings Settings(LODLimit, LODLowerLimit, MaxOctreeLeaves, WorldBounds, CameraPositions, SquaredDistances);

	NewOctree = MakeShared<FVoxelChunkOctree>(Settings, LOD);
	OldOctree = Octree;

	NumberOfLeaves = Settings.NumberOfLeaves;
	bWasCanceled = !Settings.ShouldContinue();

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
	WorkEndTime = std::chrono::high_resolution_clock::now();
}

void FAsyncOctreeBuilderTask::DoThreadedWork()
{
	DoWork();

	FScopeLock Lock(&DoneSection);
	bIsDone = true;
	if (bAutodelete)
	{
		delete this;
	}
}

void FAsyncOctreeBuilderTask::Autodelete()
{
	FScopeLock Lock(&DoneSection);
	bAutodelete = true;
	if (bIsDone)
	{
		delete this;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelChunkOctree::FVoxelChunkOctree(FVoxelChunkOctreeSettings& Settings, uint8 LOD)
	: TVoxelOctree(LOD)
	, Settings(Settings)
	, Root(this)
{
	check(LOD > 0);

	CreateChildren();

	bContinueInit = true;
	while (bContinueInit && Settings.ShouldContinue())
	{
		bContinueInit = false;
		Init();
	}
}

FVoxelChunkOctree::FVoxelChunkOctree(FVoxelChunkOctree* Parent, uint8 ChildIndex)
	: TVoxelOctree(Parent, ChildIndex)
	, Settings(Parent->Settings)
	, Root(Parent->Root)
{
	if (UNLIKELY(!Settings.ShouldContinue()))
	{
		return;
	}

	if (ShouldSubdivide())
	{
		CreateChildren();
	}
	else
	{
		Settings.NumberOfLeaves++;
	}
}

bool FVoxelChunkOctree::ShouldSubdivide() const
{
	if (LOD == 0)
	{
		return false;
	}
	if (LOD <= Settings.LODLowerLimit)
	{
		return false;
	}
	if (!GetBounds().Intersect(Settings.WorldBounds))
	{
		return false;
	}
	if (LOD > Settings.LODLimit)
	{
		return true;
	}

	for (auto& CameraPosition : Settings.CameraPositions)
	{
		if (GetBounds().IsInside(CameraPosition))
		{
			return true;
		}
	}

	uint64 SquaredDistance = MAX_uint64;
	for (auto& CameraPosition : Settings.CameraPositions)
	{
		SquaredDistance = FMath::Min<uint64>(SquaredDistance, OctreeBounds.ComputeSquaredDistanceFromBoxToPoint<uint64>(CameraPosition));
	}
	if (SquaredDistance < Settings.SquaredDistances[LOD])
	{
		return true;
	}

	return false;
}

void FVoxelChunkOctree::GetLeavesBounds(TSet<FIntBox>& InBounds) const
{
	if (IsLeaf())
	{
		if (GetBounds().Intersect(Settings.WorldBounds))
		{
			InBounds.Add(GetBounds());
		}
	}
	else
	{
		for (auto& Child : GetChildren())
		{
			Child->GetLeavesBounds(InBounds);
		}
	}
}

void FVoxelChunkOctree::GetLeavesTransitionsMasks(TMap<FIntBox, uint8>& TransitionsMasks) const
{
	if (IsLeaf())
	{
		if (GetBounds().Intersect(Settings.WorldBounds))
		{
			uint8 TransitionsMask = 0;
			for (auto& Direction : { XMin, XMax, YMin, YMax, ZMin, ZMax })
			{
				FVoxelChunkOctree* AdjacentChunk = GetAdjacentChunk(Direction);
				if (AdjacentChunk && AdjacentChunk->LOD < LOD)
				{
					TransitionsMask |= Direction;
				}
			}
			TransitionsMasks.Add(GetBounds(), TransitionsMask);
		}
	}
	else
	{
		for (auto& Child : GetChildren())
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
	if (UNLIKELY(!Settings.ShouldContinue()))
	{
		return;
	}

	if (IsLeaf())
	{
		for (auto& Direction : { XMin, XMax, YMin, YMax, ZMin, ZMax })
		{
			FVoxelChunkOctree* AdjacentChunk = GetAdjacentChunk(Direction);
			while (AdjacentChunk && AdjacentChunk->LOD > LOD + 1 && AdjacentChunk->GetBounds().Intersect(Settings.WorldBounds))
			{
				Settings.NumberOfLeaves += 3; // 4 new childs, 1 less leaf
				AdjacentChunk->CreateChildren();
				AdjacentChunk = GetAdjacentChunk(Direction);
				Root->bContinueInit = true;
			}
		}
	}
	else
	{
		for (auto& Child : GetChildren())
		{
			Child->Init();
		}
	}
}
