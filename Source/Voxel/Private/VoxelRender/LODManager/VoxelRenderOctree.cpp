// Copyright 2020 Phyronnaz

#include "VoxelRenderOctree.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelMessages.h"
#include "Async/Async.h"

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Render Octrees Count"), STAT_VoxelRenderOctreesCount, STATGROUP_VoxelCounters);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelRenderOctreesMemory);

static TAutoConsoleVariable<int32> CVarMaxRenderOctreeChunks(
	TEXT("voxel.renderer.MaxRenderOctreeChunks"),
	1000000,
	TEXT("Max render octree chunks. Allows to stop the creation of the octree before it gets too big & freezes your computer"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarLogRenderOctreeBuildTime(
	TEXT("voxel.renderer.LogRenderOctreeBuildTime"),
	0,
	TEXT("If true, will log the render octree build times"),
	ECVF_Default);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelRenderOctreeAsyncBuilder::FVoxelRenderOctreeAsyncBuilder(uint8 OctreeDepth, const FVoxelIntBox& WorldBounds)
	: FVoxelAsyncWork(STATIC_FNAME("Render Octree Build"), 1e9)
	, OctreeDepth(OctreeDepth)
	, WorldBounds(WorldBounds)
{
	SetIsDone(true);
}

void FVoxelRenderOctreeAsyncBuilder::Init(const FVoxelRenderOctreeSettings& InOctreeSettings, TVoxelSharedPtr<FVoxelRenderOctree> InOctree)
{
	VOXEL_FUNCTION_COUNTER();

	OctreeSettings = InOctreeSettings;
	OldOctree = InOctree;

	SetIsDone(false);
	Counter = FPlatformTime::Seconds();
	Log = "Render octree build stats:";
}

#define LOG_TIME_IMPL(Name, Counter) Log += "\n\t" Name ": " + FString::SanitizeFloat((FPlatformTime::Seconds() - Counter) * 1000.f) + "ms"; Counter = FPlatformTime::Seconds();
#define LOG_TIME(Name) LOG_TIME_IMPL("\t" Name, Counter)

void FVoxelRenderOctreeAsyncBuilder::ReportBuildTime()
{
	VOXEL_FUNCTION_COUNTER();
	
	LOG_TIME("Waiting for game thread");
	
	if (CVarLogRenderOctreeBuildTime.GetValueOnGameThread())
	{
		LOG_VOXEL(Log, TEXT("%s"), *Log);
	}

	if (bTooManyChunks)
	{
		FVoxelMessages::Error(FString::Printf(TEXT(
			"Render octree update was stopped!\n" 
			"Max render octree chunks count reached: voxel.renderer.MaxRenderOctreeChunks < %d.\n"
			"This is caused by too demanding LOD settings.\n"
			"You can try the following: \n"
			"- reduce World Size\n"
			"- increase Max LOD\n"
			"- reduce invokers distances"), NumberOfChunks));
	}
}

void FVoxelRenderOctreeAsyncBuilder::DoWork()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	LOG_TIME_IMPL("Waiting in thread pool", Counter);

	double WorkStartTime = FPlatformTime::Seconds();
	
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Deleting previous octree");
		OctreeToDelete.Reset();
		LOG_TIME("Deleting previous octree");
	}

	{
		VOXEL_ASYNC_SCOPE_COUNTER("Resetting arrays");
		ChunkUpdates.Reset();
		NewOctree.Reset();
		LOG_TIME("Resetting arrays");
	}
	
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Cloning octree");
		NewOctree = OldOctree.IsValid() ? MakeVoxelShared<FVoxelRenderOctree>(&*OldOctree) : MakeVoxelShared<FVoxelRenderOctree>(OctreeDepth);
		LOG_TIME("Cloning octree");
	}
	
	{
		VOXEL_ASYNC_SCOPE_COUNTER("ResetDivisionType");
		NewOctree->ResetDivisionType();
		LOG_TIME("ResetDivisionType");
	}

	bool bChanged;
	{
		VOXEL_ASYNC_SCOPE_COUNTER("UpdateSubdividedByDistance");
		bChanged = NewOctree->UpdateSubdividedByDistance(OctreeSettings);
		LOG_TIME("UpdateSubdividedByDistance");
		Log += "; Need to recompute neighbors: " + FString(bChanged ? "true" : "false");
	}

	if (bChanged)
	{
		VOXEL_ASYNC_SCOPE_COUNTER("UpdateSubdividedByNeighbors");
		int32 UpdateSubdividedByNeighborsCounter = 0;
		while (NewOctree->UpdateSubdividedByNeighbors(OctreeSettings)) { UpdateSubdividedByNeighborsCounter++; }
		LOG_TIME("UpdateSubdividedByNeighbors");
		Log += "; Iterations: " + FString::FromInt(UpdateSubdividedByNeighborsCounter);
	}
	else
	{
		VOXEL_ASYNC_SCOPE_COUNTER("ReuseOldNeighbors");
		NewOctree->ReuseOldNeighbors();
	}
	
	{
		VOXEL_ASYNC_SCOPE_COUNTER("UpdateSubdividedByOthers");
		NewOctree->UpdateSubdividedByOthers(OctreeSettings);
		LOG_TIME("UpdateSubdividedByOthers");
	}
	
	{
		VOXEL_ASYNC_SCOPE_COUNTER("DeleteChunks");
		NewOctree->DeleteChunks(ChunkUpdates);
		LOG_TIME("DeleteChunks");
	}
	
	{
		VOXEL_ASYNC_SCOPE_COUNTER("GetUpdates");
		NewOctree->GetUpdates(NewOctree->UpdateIndex + 1, bChanged, OctreeSettings, ChunkUpdates);
		LOG_TIME("GetUpdates");
	}
	
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Sort By LODs");
		// Make sure that LOD 0 chunks are processed first
		ChunkUpdates.Sort([](const auto& A, const auto& B) { return A.LOD < B.LOD; });
		LOG_TIME("Sort By LODs");
	}

	if (OldOctree.IsValid())
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Find previous chunks");
		for (auto& ChunkUpdate : ChunkUpdates)
		{
			if (ChunkUpdate.NewSettings.bVisible && !ChunkUpdate.OldSettings.bVisible)
			{
				OldOctree->GetVisibleChunksOverlappingBounds(ChunkUpdate.Bounds, ChunkUpdate.PreviousChunks);
			}
		}
	}
	LOG_TIME("Find previous chunks");
	
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Deleting old octree");
		OldOctree.Reset();
		LOG_TIME("Deleting old octree");
	}

	NumberOfChunks = NewOctree->CurrentChunksCount;
	bTooManyChunks = NewOctree->IsCanceled();

	if (bTooManyChunks)
	{
		NewOctree.Reset();
	}

	LOG_TIME_IMPL("Total time working", WorkStartTime);
}

uint32 FVoxelRenderOctreeAsyncBuilder::GetPriority() const
{
	return 0;
}

#undef LOG_TIME

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define CHECK_MAX_CHUNKS_COUNT_IMPL(ReturnValue) if (IsCanceled()) { return ReturnValue; }
#define CHECK_MAX_CHUNKS_COUNT() CHECK_MAX_CHUNKS_COUNT_IMPL(;)
#define CHECK_MAX_CHUNKS_COUNT_BOOL() CHECK_MAX_CHUNKS_COUNT_IMPL(false)

FVoxelRenderOctree::FVoxelRenderOctree(uint8 LOD)
	: TSimpleVoxelOctree(LOD)
	, Root(this)
	, ChunkId(GetId())
	, OctreeBounds(GetBounds())
{
	check(LOD > 0);
	check(ChunkId <= Root->RootIdCounter);
	Root->CurrentChunksCount++;

	INC_DWORD_STAT_BY(STAT_VoxelRenderOctreesCount, 1);
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelRenderOctreesMemory, sizeof(FVoxelRenderOctree));
}

FVoxelRenderOctree::FVoxelRenderOctree(const FVoxelRenderOctree* Source)
	: TSimpleVoxelOctree(Source->Height)
	, RootIdCounter(Source->RootIdCounter)
	, Root(this)
	, ChunkId(Source->ChunkId)
	, OctreeBounds(GetBounds())
	, UpdateIndex(Source->UpdateIndex)
{
	check(ChunkId <= Root->RootIdCounter);
	Root->CurrentChunksCount++;
	ChunkSettings = Source->ChunkSettings;
	if (Source->HasChildren())
	{
		CreateChildren(Source->GetChildren());
	}

	INC_DWORD_STAT_BY(STAT_VoxelRenderOctreesCount, 1);
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelRenderOctreesMemory, sizeof(FVoxelRenderOctree));
}


FVoxelRenderOctree::FVoxelRenderOctree(const FVoxelRenderOctree& Parent, uint8 ChildIndex)
	: TSimpleVoxelOctree(Parent, ChildIndex)
	, Root(Parent.Root)
	, ChunkId(GetId())
	, OctreeBounds(GetBounds())
	, UpdateIndex(Parent.UpdateIndex)
{
	check(ChunkId <= Root->RootIdCounter);
	Root->CurrentChunksCount++;

	INC_DWORD_STAT_BY(STAT_VoxelRenderOctreesCount, 1);
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelRenderOctreesMemory, sizeof(FVoxelRenderOctree));
}


FVoxelRenderOctree::FVoxelRenderOctree(const FVoxelRenderOctree& Parent, uint8 ChildIndex, const ChildrenArray& SourceChildren)
	: TSimpleVoxelOctree(Parent, ChildIndex)
	, Root(Parent.Root)
	, ChunkId(SourceChildren[ChildIndex].ChunkId)
	, OctreeBounds(GetBounds())
	, UpdateIndex(Parent.UpdateIndex)
{
	Root->CurrentChunksCount++;

	auto& Source = SourceChildren[ChildIndex];
	ChunkSettings = Source.ChunkSettings;
	if (Source.HasChildren())
	{
		CreateChildren(Source.GetChildren());
	}

	INC_DWORD_STAT_BY(STAT_VoxelRenderOctreesCount, 1);
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelRenderOctreesMemory, sizeof(FVoxelRenderOctree));
}

FVoxelRenderOctree::~FVoxelRenderOctree()
{
	Root->CurrentChunksCount--;
	DEC_DWORD_STAT_BY(STAT_VoxelRenderOctreesCount, 1);
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelRenderOctreesMemory, sizeof(FVoxelRenderOctree));
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderOctree::ResetDivisionType()
{
	ChunkSettings.OldDivisionType = ChunkSettings.DivisionType;
	ChunkSettings.DivisionType = EDivisionType::Uninitialized;

	if (!!HasChildren())
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
		
		if (!HasChildren())
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
		
		if (!HasChildren())
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

	if (!!HasChildren())
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

		if (!HasChildren())
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

void FVoxelRenderOctree::DeleteChunks(TArray<FVoxelChunkUpdate>& ChunkUpdates)
{
	CHECK_MAX_CHUNKS_COUNT();

	if (ChunkSettings.DivisionType == EDivisionType::Uninitialized)
	{		
		if (HasChildren())
		{
			for (auto& Child : GetChildren())
			{
				ensure(Child.ChunkSettings.DivisionType == EDivisionType::Uninitialized);
				
				Child.DeleteChunks(ChunkUpdates);
				
				if (Child.ChunkSettings.Settings.HasRenderChunk())
				{
					//ensureVoxelSlowNoSideEffects(!ChunkUpdates.FindByPredicate([&](const FVoxelChunkUpdate& ChunkUpdate) { return ChunkUpdate.Id == Child.ChunkId; }));
					ChunkUpdates.Emplace(
						FVoxelChunkUpdate
						{
							Child.ChunkId,
							Child.Height,
							Child.OctreeBounds,
							Child.ChunkSettings.Settings,
							{},
							{}
						});
				}
			}
			DestroyChildren();
		}
	}
	else
	{
		for (auto& Child : GetChildren())
		{
			Child.DeleteChunks(ChunkUpdates);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderOctree::GetUpdates(
	uint32 InUpdateIndex,
	bool bRecomputeTransitionMasks,
	const FVoxelRenderOctreeSettings& Settings,
	TArray<FVoxelChunkUpdate>& ChunkUpdates,
	bool bInVisible)
{
	CHECK_MAX_CHUNKS_COUNT();

	UpdateIndex++;
	check(UpdateIndex == InUpdateIndex);

	if (!OctreeBounds.Intersect(Settings.WorldBounds))
	{
		return;
	}

	FVoxelChunkSettings NewSettings{};
	
	// NOTE: we DO want bEnableRender = false to disable VisibleChunks settings
	NewSettings.bVisible = Settings.bEnableRender && Height <= Settings.ChunksCullingLOD && bInVisible;

	if (!HasChildren())
	{
		check(ChunkSettings.DivisionType == EDivisionType::Uninitialized);
	}
	else
	{
		check(ChunkSettings.DivisionType != EDivisionType::Uninitialized);
		bool bChildrenVisible;
		if (ChunkSettings.DivisionType == EDivisionType::ByDistance || ChunkSettings.DivisionType == EDivisionType::ByNeighbors)
		{
			// There are visible children
			NewSettings.bVisible = false;
			bChildrenVisible = true;
		}
		else
		{
			check(ChunkSettings.DivisionType == EDivisionType::ByOthers);
			bChildrenVisible = false;
		}

		for (auto& Child : GetChildren())
		{
			Child.GetUpdates(UpdateIndex, bRecomputeTransitionMasks, Settings, ChunkUpdates, bChildrenVisible);
		}
	}

	NewSettings.bEnableCollisions =
		Settings.bEnableCollisions &&
		((Height == 0 &&
			IsInvokerInRange(Settings.Invokers,
				[](const FVoxelInvokerSettings& Invoker) { return Invoker.bUseForCollisions; },
				[](const FVoxelInvokerSettings& Invoker) { return Invoker.CollisionsBounds; })
		 )
		 ||
		 (NewSettings.bVisible && Settings.bComputeVisibleChunksCollisions && Height <= Settings.VisibleChunksCollisionsMaxLOD)
	    );
		
	NewSettings.bEnableNavmesh = 
		Settings.bEnableNavmesh &&
		((Height == 0 &&
			IsInvokerInRange(Settings.Invokers,
				[](const FVoxelInvokerSettings& Invoker) { return Invoker.bUseForNavmesh; },
				[](const FVoxelInvokerSettings& Invoker) { return Invoker.NavmeshBounds; })
		)
		||
		(NewSettings.bVisible && Settings.bComputeVisibleChunksNavmesh && Height <= Settings.VisibleChunksNavmeshMaxLOD)
		);

	check(NewSettings.TransitionsMask == 0);
	if (NewSettings.HasRenderChunk())
	{
		if (NewSettings.bVisible && Settings.bEnableTransitions)
		{
			if (bRecomputeTransitionMasks)
			{
				for (int32 DirectionIndex = 0; DirectionIndex < 6; DirectionIndex++)
				{
					const auto Direction = EVoxelDirectionFlag::Type(1 << DirectionIndex);
					const FVoxelRenderOctree* AdjacentChunk = GetVisibleAdjacentChunk(Direction, 0);
					if (AdjacentChunk && AdjacentChunk->OctreeBounds.Intersect(Settings.WorldBounds))
					{
						check(
							(AdjacentChunk->Height == Height - 1) ||
							(AdjacentChunk->Height == Height) ||
							(AdjacentChunk->Height == Height + 1)
						);
						if (Settings.bInvertTransitions ? (AdjacentChunk->Height > Height) : (AdjacentChunk->Height < Height))
						{
							NewSettings.TransitionsMask |= Direction;
						}
					}
				}
			}
			else
			{
				NewSettings.TransitionsMask = ChunkSettings.Settings.TransitionsMask;
			}
		}
	}
	
	if (ChunkSettings.Settings != NewSettings && (ChunkSettings.Settings.HasRenderChunk() || NewSettings.HasRenderChunk()))
	{
		// Too slow ensureVoxelSlowNoSideEffects(!ChunkUpdates.FindByPredicate([&](const FVoxelChunkUpdate& ChunkUpdate) { return ChunkUpdate.Id == ChunkId; }));
		ChunkUpdates.Emplace(
			FVoxelChunkUpdate
			{
				ChunkId,
				Height,
				OctreeBounds,
				ChunkSettings.Settings,
				NewSettings,
				{}
			});
	}
	
	ChunkSettings.Settings = NewSettings;
}

void FVoxelRenderOctree::GetChunksToUpdateForBounds(const FVoxelIntBox& Bounds, TArray<uint64>& ChunksToUpdate, const FVoxelOnChunkUpdate& OnChunkUpdate) const
{
	if (!OctreeBounds.Intersect(Bounds))
	{
		return;
	}

	if (ChunkSettings.Settings.HasRenderChunk())
	{
		OnChunkUpdate.Broadcast(OctreeBounds);
		ChunksToUpdate.Add(ChunkId);
	}

	if (!!HasChildren())
	{
		for (auto& Child : GetChildren())
		{
			Child.GetChunksToUpdateForBounds(Bounds, ChunksToUpdate, OnChunkUpdate);
		}
	}
}


void FVoxelRenderOctree::GetVisibleChunksOverlappingBounds(const FVoxelIntBox& Bounds, TArray<uint64, TInlineAllocator<8>>& VisibleChunks) const
{
	if (!OctreeBounds.Intersect(Bounds))
	{
		return;
	}

	if (ChunkSettings.Settings.bVisible)
	{
		VisibleChunks.Add(ChunkId);
	}

	if (!!HasChildren())
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
	if (Height == 0)
	{
		return false;
	}
	if (!OctreeBounds.Intersect(Settings.WorldBounds))
	{
		return false;
	}
	if (Height <= Settings.MinLOD)
	{
		return false;
	}
	if (Height > Settings.MaxLOD)
	{
		return true;
	}

	for (auto& Invoker : Settings.Invokers)
	{
		if (Invoker.bUseForLOD && OctreeBounds.Intersect(Invoker.LODBounds) && Height > Invoker.LODToSet)
		{
			return true;
		}
	}

	return false;
}


bool FVoxelRenderOctree::ShouldSubdivideByNeighbors(const FVoxelRenderOctreeSettings& Settings) const
{
	if (Height == 0)
	{
		return false;
	}
	if (!OctreeBounds.Intersect(Settings.WorldBounds))
	{
		return false;
	}
	for (int32 DirectionIndex = 0; DirectionIndex < 6; DirectionIndex++)
	{
		const auto Direction = EVoxelDirectionFlag::Type(1 << DirectionIndex);
		for (int32 Index = 0; Index < 4; Index++) // Iterate the 4 adjacent subdivided chunks
		{
			const FVoxelRenderOctree* AdjacentChunk = GetVisibleAdjacentChunk(Direction, Index);
			if (!AdjacentChunk)
			{
				continue;
			}

			if (AdjacentChunk->Height + 1 < Height)
			{
				return true;
			}
			if (AdjacentChunk->Height >= Height)
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
	if (Height == 0)
	{
		return false;
	}
	if (!OctreeBounds.Intersect(Settings.WorldBounds))
	{
		return false;
	}

	if (Settings.bEnableCollisions && IsInvokerInRange(Settings.Invokers,
		[](const FVoxelInvokerSettings& Invoker) { return Invoker.bUseForCollisions; },
		[](const FVoxelInvokerSettings& Invoker) { return Invoker.CollisionsBounds; }))
	{
		return true;
	}
	if (Settings.bEnableNavmesh && IsInvokerInRange(Settings.Invokers,
		[](const FVoxelInvokerSettings& Invoker) { return Invoker.bUseForNavmesh; },
		[](const FVoxelInvokerSettings& Invoker) { return Invoker.NavmeshBounds; }))
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

const FVoxelRenderOctree* FVoxelRenderOctree::GetVisibleAdjacentChunk(EVoxelDirectionFlag::Type Direction, int32 Index) const
{
	const int32 HalfSize = Size() / 2;
	const int32 HalfHalfSize = Size() / 4;

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
	case EVoxelDirectionFlag::XMin:
		P = Position + FIntVector(-S, X, Y);
		break;
	case EVoxelDirectionFlag::XMax:
		P = Position + FIntVector(S, X, Y);
		break;
	case EVoxelDirectionFlag::YMin:
		P = Position + FIntVector(X, -S, Y);
		break;
	case EVoxelDirectionFlag::YMax:
		P = Position + FIntVector(X, S, Y);
		break;
	case EVoxelDirectionFlag::ZMin:
		P = Position + FIntVector(X, Y, -S);
		break;
	case EVoxelDirectionFlag::ZMax:
		P = Position + FIntVector(X, Y, S);
		break;
	default:
		check(false);
		P = FIntVector::ZeroValue;
	}

	if (Root->OctreeBounds.Contains(P))
	{
		const FVoxelRenderOctree* Ptr = Root;

		while (IsVisibleParent(Ptr))
		{
			Ptr = &Ptr->GetChild(P);
		}

		check(Ptr->OctreeBounds.Contains(P));

		return Ptr;
	}
	else
	{
		return nullptr;
	}
}

template<typename T1, typename T2>
bool FVoxelRenderOctree::IsInvokerInRange(const TArray<FVoxelInvokerSettings>& Invokers, T1 SelectInvoker, T2 GetInvokerBounds) const
{
	for (auto& Invoker : Invokers)
	{
		if (SelectInvoker(Invoker))
		{
			if (OctreeBounds.Intersect(GetInvokerBounds(Invoker)))
			{
				return true;
			}
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

uint64 FVoxelRenderOctree::GetId()
{
	return ++Root->RootIdCounter;
}