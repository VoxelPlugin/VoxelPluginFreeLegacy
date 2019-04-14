// Copyright 2019 Phyronnaz

#include "VoxelRenderOctree.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "Async/Async.h"
#include "VoxelMathUtilities.h"
#include "VoxelRender/IVoxelLODManager.h"

DECLARE_MEMORY_STAT(TEXT("Voxel Render Octrees Memory"), STAT_VoxelRenderOctreesMemory, STATGROUP_VoxelMemory);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Render Octrees Count"), STAT_VoxelRenderOctreesCount, STATGROUP_VoxelMemory);

static TAutoConsoleVariable<int32> CVarMaxRenderOctreeChunks(
	TEXT("voxel.MaxRenderOctreeChunks"),
	1000000,
	TEXT("Max render octree chunks. Allows to stop the creation of the octree before it gets too big & freezes your computer"),
	ECVF_Default);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelRenderOctreeAsyncBuilder::FVoxelRenderOctreeAsyncBuilder(uint8 OctreeDepth, const FIntBox& WorldBounds)
	: OctreeDepth(OctreeDepth)
	, WorldBounds(WorldBounds)
{
}

void FVoxelRenderOctreeAsyncBuilder::Init(const FVoxelRenderOctreeSettings& InOctreeSettings, TSharedPtr<FVoxelRenderOctree, ESPMode::ThreadSafe> InOctree)
{
	OctreeSettings = InOctreeSettings;
	OldOctree = InOctree;

	bIsDone = false;
	Counter = FPlatformTime::Seconds();
	Log = "Render octree build stats:";
}

#define LOG_TIME_IMPL(Name, Counter) Log += "\n\t" Name ": " + FString::SanitizeFloat((FPlatformTime::Seconds() - Counter) * 1000.f) + "ms"; Counter = FPlatformTime::Seconds();
#define LOG_TIME(Name) LOG_TIME_IMPL(Name, Counter)

void FVoxelRenderOctreeAsyncBuilder::ReportBuildTime(FVoxelDebugManager& DebugManager)
{
	LOG_TIME("Waiting for game thread");
	DebugManager.ReportRenderOctreeBuild(Log, NumberOfChunks, bTooManyChunks);
}

void FVoxelRenderOctreeAsyncBuilder::DoWork()
{
	LOG_TIME("Waiting in thread pool");

	double WorkStartTime = FPlatformTime::Seconds();
	
	ChunksToAdd.Reset();
	ChunksToUpdate.Reset();
	ChunksToRemove.Reset();
	TransitionsToUpdate.Reset();
	ChunksWithLOD0Collisions.Reset();
	NewOctree.Reset();
	LOG_TIME("\tResetting arrays");

	NewOctree = OldOctree.IsValid() ? MakeShared<FVoxelRenderOctree, ESPMode::ThreadSafe>(&*OldOctree) : MakeShared<FVoxelRenderOctree, ESPMode::ThreadSafe>(OctreeDepth);
	LOG_TIME("\tCloning octree");

	NewOctree->ResetDivisionType();
	LOG_TIME("\tResetDivisionType");

	bool bChanged = NewOctree->UpdateSubdividedByDistance(OctreeSettings);
	LOG_TIME("\tUpdateSubdividedByDistance");
	Log += "; Need to recompute neighbors: " + FString(bChanged ? "true" : "false");

	if (bChanged)
	{
		int32 UpdateSubdividedByNeighborsCounter = 0;
		while (NewOctree->UpdateSubdividedByNeighbors(OctreeSettings)) { UpdateSubdividedByNeighborsCounter++; }
		LOG_TIME("\tUpdateSubdividedByNeighbors");
		Log += "; Iterations: " + FString::FromInt(UpdateSubdividedByNeighborsCounter);
	}
	else
	{
		NewOctree->ReuseOldNeighbors();
	}

	NewOctree->UpdateSubdividedByOthers(OctreeSettings);
	LOG_TIME("\tUpdateSubdividedByOthers");

	NewOctree->DeleteChunks(ChunksToRemove);
	LOG_TIME("\tDeleteChunks");

	NewOctree->GetUpdates(bChanged, OctreeSettings, ChunksToAdd, ChunksToUpdate, ChunksToRemove, TransitionsToUpdate, ChunksWithLOD0Collisions);
	LOG_TIME("\tGetUpdates");

	if(OldOctree.IsValid())
	{
		for (auto& ChunkToAdd : ChunksToAdd)
		{
			OldOctree->GetVisibleChunksOverlappingBounds(ChunkToAdd.Bounds, ChunkToAdd.PreviousChunks);
		}
		for (auto& ChunkToUpdate : ChunksToUpdate)
		{
			if (ChunkToUpdate.NewSettings.bVisible && !ChunkToUpdate.OldSettings.bVisible)
			{
				OldOctree->GetVisibleChunksOverlappingBounds(ChunkToUpdate.Bounds, ChunkToUpdate.PreviousChunks);
			}
		}
	}
	LOG_TIME("\tFind previous chunks");

	OldOctree = NewOctree;
	LOG_TIME("\tDeleting old octree");

	NumberOfChunks = NewOctree->CurrentChunksCount;
	bTooManyChunks = NewOctree->IsCanceled();

	if (bTooManyChunks)
	{
		NewOctree.Reset();
	}

	auto DelegateCopy = Delegate;
	AsyncTask(ENamedThreads::GameThread, [DelegateCopy]() { DelegateCopy.ExecuteIfBound(); });

	LOG_TIME_IMPL("Total time working", WorkStartTime);
}

#undef LOG_TIME

void FVoxelRenderOctreeAsyncBuilder::DoThreadedWork()
{
	DoWork();

	FScopeLock Lock(&DoneSection);
	bIsDone = true;
	if (bAutodelete)
	{
		delete this;
	}
}


void FVoxelRenderOctreeAsyncBuilder::Abandon()
{
	FScopeLock Lock(&DoneSection);
	bIsDone = true;
	if (bAutodelete)
	{
		delete this;
	}
}

void FVoxelRenderOctreeAsyncBuilder::Autodelete()
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

#define CHECK_MAX_CHUNKS_COUNT_IMPL(ReturnValue) if (IsCanceled()) { return ReturnValue; }
#define CHECK_MAX_CHUNKS_COUNT() CHECK_MAX_CHUNKS_COUNT_IMPL(;)
#define CHECK_MAX_CHUNKS_COUNT_BOOL() CHECK_MAX_CHUNKS_COUNT_IMPL(false)

FVoxelRenderOctree::FVoxelRenderOctree(uint8 LOD)
	: TVoxelOctree(LOD)
	, ChunkId(GetId())
	, Root(this)
{
	check(LOD > 0);
	Root->CurrentChunksCount++;

	INC_DWORD_STAT_BY(STAT_VoxelRenderOctreesCount, 1);
	INC_MEMORY_STAT_BY(STAT_VoxelRenderOctreesMemory, sizeof(FVoxelRenderOctree));
}

FVoxelRenderOctree::FVoxelRenderOctree(const FVoxelRenderOctree* Source)
	: TVoxelOctree(Source->LOD)
	, ChunkId(Source->ChunkId)
	, Root(this)
{
	Root->CurrentChunksCount++;
	ChunkSettings = Source->ChunkSettings;
	if (!Source->IsLeaf())
	{
		CreateChildren(Source->GetChildren());
	}

	INC_DWORD_STAT_BY(STAT_VoxelRenderOctreesCount, 1);
	INC_MEMORY_STAT_BY(STAT_VoxelRenderOctreesMemory, sizeof(FVoxelRenderOctree));
}


FVoxelRenderOctree::FVoxelRenderOctree(FVoxelRenderOctree* Parent, uint8 ChildIndex)
	: TVoxelOctree(Parent, ChildIndex)
	, ChunkId(GetId())
	, Root(Parent->Root)
{
	Root->CurrentChunksCount++;

	INC_DWORD_STAT_BY(STAT_VoxelRenderOctreesCount, 1);
	INC_MEMORY_STAT_BY(STAT_VoxelRenderOctreesMemory, sizeof(FVoxelRenderOctree));
}


FVoxelRenderOctree::FVoxelRenderOctree(FVoxelRenderOctree* Parent, uint8 ChildIndex, const FChildrenArray& SourceChildren)
	: TVoxelOctree(Parent, ChildIndex)
	, ChunkId(SourceChildren[ChildIndex].ChunkId)
	, Root(Parent->Root)
{
	Root->CurrentChunksCount++;

	auto& Source = SourceChildren[ChildIndex];
	ChunkSettings = Source.ChunkSettings;
	if (!Source.IsLeaf())
	{
		CreateChildren(Source.GetChildren());
	}

	INC_DWORD_STAT_BY(STAT_VoxelRenderOctreesCount, 1);
	INC_MEMORY_STAT_BY(STAT_VoxelRenderOctreesMemory, sizeof(FVoxelRenderOctree));
}

FVoxelRenderOctree::~FVoxelRenderOctree()
{
	Root->CurrentChunksCount--;
	DEC_DWORD_STAT_BY(STAT_VoxelRenderOctreesCount, 1);
	DEC_MEMORY_STAT_BY(STAT_VoxelRenderOctreesMemory, sizeof(FVoxelRenderOctree));
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderOctree::ResetDivisionType()
{
	ChunkSettings.OldDivisionType = ChunkSettings.DivisionType;
	ChunkSettings.DivisionType = EDivisionType::Uninitialized;

	if (!IsLeaf())
	{
		for (auto& Child : GetChildren())
		{
			Child.ResetDivisionType();
		}
	}
}

bool FVoxelRenderOctree::UpdateSubdividedByDistance(const FVoxelRenderOctreeSettings& Settings)
{
	CHECK_MAX_CHUNKS_COUNT_BOOL();
	
	if (ShouldSubdivideByDistance(Settings))
	{
		ChunkSettings.DivisionType = EDivisionType::ByDistance;
		
		if (IsLeaf())
		{
			CreateChildren();
		}
		
		bool bChanged = ChunkSettings.OldDivisionType != EDivisionType::ByDistance;
		for (auto& Child : GetChildren())
		{
			bChanged |= Child.UpdateSubdividedByDistance(Settings);
		}
	
		return bChanged;
	}
	else
	{
		return ChunkSettings.OldDivisionType == EDivisionType::ByDistance;
	}
}

bool FVoxelRenderOctree::UpdateSubdividedByNeighbors(const FVoxelRenderOctreeSettings& Settings)
{
	CHECK_MAX_CHUNKS_COUNT_BOOL();

	bool bShouldContinue = false;

	if (ChunkSettings.DivisionType == EDivisionType::Uninitialized && ShouldSubdivideByNeighbors(Settings))
	{
		ChunkSettings.DivisionType = EDivisionType::ByNeighbors;
		
		if (IsLeaf())
		{
			CreateChildren();
		}

		bShouldContinue = true;
	}

	if (ChunkSettings.DivisionType != EDivisionType::Uninitialized)
	{
		for (auto& Child : GetChildren())
		{
			bShouldContinue |= Child.UpdateSubdividedByNeighbors(Settings);
		}
	}

	return bShouldContinue;
}

void FVoxelRenderOctree::ReuseOldNeighbors()
{
	if (ChunkSettings.OldDivisionType == EDivisionType::ByNeighbors)
	{
		ChunkSettings.DivisionType = EDivisionType::ByNeighbors;
	}

	if (!IsLeaf())
	{
		for (auto& Child : GetChildren())
		{
			Child.ReuseOldNeighbors();
		}
	}
}

void FVoxelRenderOctree::UpdateSubdividedByOthers(const FVoxelRenderOctreeSettings& Settings)
{
	CHECK_MAX_CHUNKS_COUNT();

	if (ChunkSettings.DivisionType == EDivisionType::Uninitialized && ShouldSubdivideByOthers(Settings))
	{
		ChunkSettings.DivisionType = EDivisionType::ByOthers;

		if (IsLeaf())
		{
			CreateChildren();
		}
	}

	if (ChunkSettings.DivisionType != EDivisionType::Uninitialized)
	{
		for (auto& Child : GetChildren())
		{
			Child.UpdateSubdividedByOthers(Settings);
		}
	}
}

void FVoxelRenderOctree::DeleteChunks(TArray<FVoxelChunkToRemove>& ChunksToRemove)
{
	CHECK_MAX_CHUNKS_COUNT();

	if (ChunkSettings.DivisionType == EDivisionType::Uninitialized)
	{		
		if (!IsLeaf())
		{
			for (auto& Child : GetChildren())
			{
				Child.DeleteChunks(ChunksToRemove);
				auto& ChildSettings = Child.ChunkSettings.Settings;
				auto& ChildId = Child.ChunkId;

				if (ChildSettings.IsRendered())
				{
					ChunksToRemove.Add({ ChildId });
				}
			}
			DestroyChildren();
		}
	}
	else
	{
		for (auto& Child : GetChildren())
		{
			Child.DeleteChunks(ChunksToRemove);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

template<typename T1, typename T2>
inline bool IsInRange(const FVoxelRenderOctree* This, const TArray<FVoxelInvoker>& Invokers, T1 SelectInvoker, T2 SelectRange)
{
	for (auto& Invoker : Invokers)
	{
		if (SelectInvoker(Invoker))
		{
			uint64 SquaredDistance = This->OctreeBounds.ComputeSquaredDistanceFromBoxToPoint<uint64>(Invoker.Position);
			if (SquaredDistance < SelectRange(Invoker))
			{
				return true;
			}
		}
	}
	return false;
}

void FVoxelRenderOctree::GetUpdates(
	bool bRecomputeTransitionMasks,
	const FVoxelRenderOctreeSettings& Settings,
	TArray<FVoxelChunkToAdd>& ChunksToAdd,
	TArray<FVoxelChunkToUpdate>& ChunksToUpdate,
	TArray<FVoxelChunkToRemove>& ChunksToRemove,
	TArray<FVoxelTransitionsToUpdate>& TransitionsToUpdate,
	TArray<FIntBox>& ChunksWithLOD0Collisions,
	bool bInVisible)
{
	CHECK_MAX_CHUNKS_COUNT();

	if (!OctreeBounds.Intersect(Settings.WorldBounds))
	{
		return;
	}

	FVoxelRenderChunkSettings NewSettings;
	NewSettings.bVisible = Settings.bEnableRender && LOD <= Settings.ChunksCullingLOD && bInVisible;

	if (IsLeaf())
	{
		check(ChunkSettings.DivisionType == EDivisionType::Uninitialized);
	}
	else
	{
		check(ChunkSettings.DivisionType != EDivisionType::Uninitialized);
		bool bChildrenVisible;
		if (ChunkSettings.DivisionType == EDivisionType::ByDistance || ChunkSettings.DivisionType == EDivisionType::ByNeighbors)
		{
			NewSettings.bVisible = false; // There are visible children
			bChildrenVisible = true;
		}
		else
		{
			check(ChunkSettings.DivisionType == EDivisionType::ByOthers);
			bChildrenVisible = false;
		}

		for (auto& Child : GetChildren())
		{
			Child.GetUpdates(bRecomputeTransitionMasks, Settings, ChunksToAdd, ChunksToUpdate, ChunksToRemove, TransitionsToUpdate, ChunksWithLOD0Collisions, bChildrenVisible);
		}
	}

	NewSettings.bEnableCollisions =
		Settings.bEnableCollisions &&
		((LOD == 0 &&
			IsInRange(this, Settings.Invokers,
				[](auto& X) { return X.bUseForCollisions; },
				[](auto& X) { return X.SquaredCollisionsRange; })
		 )
		 ||
		 (NewSettings.bVisible && Settings.bComputeVisibleChunksCollisions && LOD <= Settings.VisibleChunksCollisionsMaxLOD)
	    );
		
	NewSettings.bEnableNavmesh = 
		Settings.bEnableNavmesh && 
		LOD == 0 && 
		IsInRange(this, Settings.Invokers, 
			[](auto& X) { return X.bUseForNavmesh; }, 
			[](auto& X) { return X.SquaredNavmeshRange; });

	NewSettings.bEnableTessellation = 
		Settings.bEnableTessellation && 
		NewSettings.bVisible && 
		IsInRange(this, Settings.Invokers, 
			[](auto& X) { return X.bUseForLODs; }, 
			[&](auto&) { return Settings.SquaredTessellationDistance; });

	NewSettings.bForceRender = 
		LOD == 0 &&
		IsInRange(this, Settings.Invokers, 
			[](auto& X) { return true; }, 
			[](auto& X) { return X.SquaredGenerationRange; });

	if (NewSettings.IsRendered())
	{
		if (ChunkSettings.Settings != NewSettings)
		{
			if (!ChunkSettings.Settings.IsRendered())
			{
				ChunksToAdd.Emplace(FVoxelChunkToAdd{ ChunkId, OctreeBounds, LOD, NewSettings, {} });
			}
			else
			{
				ChunksToUpdate.Emplace(FVoxelChunkToUpdate{ ChunkId, OctreeBounds, ChunkSettings.Settings, NewSettings, {} });
			}
		}
		if (NewSettings.bVisible && bRecomputeTransitionMasks)
		{
			uint8 TransitionsMask = 0;
			for (auto& Direction : { XMin, XMax, YMin, YMax, ZMin, ZMax })
			{
				const FVoxelRenderOctree* AdjacentChunk = GetVisibleAdjacentChunk(Direction, 0);
				if (AdjacentChunk && AdjacentChunk->OctreeBounds.Intersect(Settings.WorldBounds))
				{
					check(
						(AdjacentChunk->LOD == LOD - 1) ||
						(AdjacentChunk->LOD == LOD    ) ||
						(AdjacentChunk->LOD == LOD + 1)
					);
					if (AdjacentChunk->LOD < LOD)
					{
						TransitionsMask |= Direction;
					}
				}
			}
			if (ChunkSettings.TransitionMask != TransitionsMask)
			{
				ChunkSettings.TransitionMask = TransitionsMask;
				TransitionsToUpdate.Emplace(FVoxelTransitionsToUpdate{ ChunkId, TransitionsMask });
			}
		}
	}
	else
	{
		if (ChunkSettings.Settings.IsRendered())
		{
			ChunksToRemove.Emplace(FVoxelChunkToRemove{ ChunkId });
		}
	}

	if (LOD == 0 && NewSettings.bEnableCollisions)
	{
		ChunksWithLOD0Collisions.Add(OctreeBounds);
	}
	
	ChunkSettings.Settings = NewSettings;
}

void FVoxelRenderOctree::GetChunksToUpdateForBounds(const FIntBox& Bounds, TArray<uint64>& ChunksToUpdate) const
{
	if (!OctreeBounds.Intersect(Bounds))
	{
		return;
	}

	if (ChunkSettings.Settings.IsRendered())
	{
		ChunksToUpdate.Add(ChunkId);
	}

	if (!IsLeaf())
	{
		for (auto& Child : GetChildren())
		{
			Child.GetChunksToUpdateForBounds(Bounds, ChunksToUpdate);
		}
	}
}


void FVoxelRenderOctree::GetVisibleChunksOverlappingBounds(const FIntBox& Bounds, TArray<uint64>& VisibleChunks) const
{
	if (!OctreeBounds.Intersect(Bounds))
	{
		return;
	}

	if (ChunkSettings.Settings.bVisible)
	{
		VisibleChunks.Add(ChunkId);
	}

	if (!IsLeaf())
	{
		for (auto& Child : GetChildren())
		{
			Child.GetVisibleChunksOverlappingBounds(Bounds, VisibleChunks);
		}
	}
}

FORCEINLINE bool FVoxelRenderOctree::IsCanceled() const
{
	return Root->CurrentChunksCount >= CVarMaxRenderOctreeChunks.GetValueOnAnyThread();
}

///////////////////////////////////////////////////////////////////////////////

bool FVoxelRenderOctree::ShouldSubdivideByDistance(const FVoxelRenderOctreeSettings& Settings) const
{
	if (!Settings.bEnableRender)
	{
		return false;
	}
	if (LOD == 0)
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

	for (auto& Invoker : Settings.Invokers)
	{
		if (Invoker.bUseForLODs && GetBounds().IsInside(Invoker.Position))
		{
			return true;
		}
	}

	if (IsInRange(this, Settings.Invokers,
		[](auto& X) { return X.bUseForLODs; },
		[&](auto&) { return Settings.SquaredLODsDistances[LOD]; }))
	{
		return true;
	}

	return false;
}


bool FVoxelRenderOctree::ShouldSubdivideByNeighbors(const FVoxelRenderOctreeSettings& Settings) const
{
	if (LOD == 0)
	{
		return false;
	}
	if (!GetBounds().Intersect(Settings.WorldBounds))
	{
		return false;
	}
	for (auto& Direction : { XMin, XMax, YMin, YMax, ZMin, ZMax })
	{
		for (int32 Index = 0; Index < 4; Index++) // Iterate the 4 adjacent subdivided chunks
		{
			const FVoxelRenderOctree* AdjacentChunk = GetVisibleAdjacentChunk(Direction, Index);
			if (!AdjacentChunk)
			{
				continue;
			}

			if (AdjacentChunk->LOD + 1 < LOD)
			{
				return true;
			}
			if (AdjacentChunk->LOD >= LOD)
			{
				check(Index == 0);
				break; // No need to continue, 4 indices are the same chunk
			}
		}
	}
	return false;
}


bool FVoxelRenderOctree::ShouldSubdivideByOthers(const FVoxelRenderOctreeSettings& Settings) const
{
	if (!Settings.bEnableCollisions && !Settings.bEnableNavmesh)
	{
		return false;
	}
	if (LOD == 0)
	{
		return false;
	}
	if (!GetBounds().Intersect(Settings.WorldBounds))
	{
		return false;
	}

	if ((Settings.bEnableCollisions && IsInRange(this, Settings.Invokers,
		[](auto& X) { return X.bUseForCollisions; },
		[](auto& X) { return X.SquaredCollisionsRange; }))
		||
		(Settings.bEnableNavmesh &&	IsInRange(this, Settings.Invokers,
			[](auto& X) { return X.bUseForNavmesh; },
			[](auto& X) { return X.SquaredNavmeshRange; }))
		||
		IsInRange(this, Settings.Invokers,
			[](auto& X) { return true; },
			[](auto& X) { return X.SquaredGenerationRange; }))
	{
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////

inline bool IsVisibleParent(const FVoxelRenderOctree* Chunk)
{
	return Chunk->ChunkSettings.DivisionType == FVoxelRenderOctree::EDivisionType::ByDistance || Chunk->ChunkSettings.DivisionType == FVoxelRenderOctree::EDivisionType::ByNeighbors;
}

const FVoxelRenderOctree* FVoxelRenderOctree::GetVisibleAdjacentChunk(EVoxelDirection Direction, int32 Index) const
{
	int32 HalfSize = Size() / 2;
	int32 HalfHalfSize = Size() / 4;

	int32 S = HalfSize + HalfHalfSize; // Size / 2: on the border; Size / 4: center of child chunk
	int32 X, Y;
	if (Index & 0x1)
	{
		X = -HalfHalfSize;
	}
	else
	{
		X = HalfHalfSize;
	}
	if (Index & 0x2)
	{
		Y = -HalfHalfSize;
	}
	else
	{
		Y = HalfHalfSize;
	}

	FIntVector P;
	switch (Direction)
	{
	case XMin:
		P = Position + FIntVector(-S, X, Y);
		break;
	case XMax:
		P = Position + FIntVector(S, X, Y);
		break;
	case YMin:
		P = Position + FIntVector(X, -S, Y);
		break;
	case YMax:
		P = Position + FIntVector(X, S, Y);
		break;
	case ZMin:
		P = Position + FIntVector(X, Y, -S);
		break;
	case ZMax:
		P = Position + FIntVector(X, Y, S);
		break;
	default:
		check(false);
		P = FIntVector::ZeroValue;
	}

	if (Root->IsInOctree(P))
	{
		const FVoxelRenderOctree* Ptr = Root;

		while (IsVisibleParent(Ptr))
		{
			Ptr = &Ptr->GetChild(P);
		}

		check(Ptr->IsInOctree(P));

		return Ptr;
	}
	else
	{
		return nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////

uint64 FVoxelRenderOctree::GetId()
{
	static uint64 LocalId = 0;
	return LocalId++;
}