// Copyright 2018 Phyronnaz

#include "VoxelData/VoxelData.h"
#include "VoxelLogStatDefinitions.h"
#include "VoxelData/VoxelDataOctree.h"
#include "VoxelSave.h"
#include "VoxelDiff.h"
#include "VoxelWorldGenerator.h"
#include "Algo/Reverse.h"
#include "Misc/ScopeLock.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelData::LoadFromDiffQueues"), STAT_VoxelData_LoadFromDiffQueues, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelData::LoadFromDiffQueues::BeginSet"), STAT_VoxelData_LoadFromDiffQueues_BeginSet, STATGROUP_Voxel);

void FVoxelData::BeginSet(const FIntBox& Box, TArray<FVoxelId>& OutOctrees)
{
	OutOctrees.Empty();

	Octree->LockTransactions();
	Octree->BeginSet(Box, OutOctrees, 1e9);

	Algo::Reverse(OutOctrees);
}

bool FVoxelData::TryBeginSet(const FIntBox& Box, int MicroSeconds, TArray<FVoxelId>& OutOctrees)
{
	OutOctrees.Empty();

	Octree->LockTransactions();

	bool bSuccess = Octree->BeginSet(Box, OutOctrees, MicroSeconds);

	Algo::Reverse(OutOctrees);

	if (!bSuccess)
	{
		EndSet(OutOctrees);
	}

	return bSuccess;
}

void FVoxelData::EndSet(TArray<FVoxelId>& LockedOctrees)
{
	Octree->EndSet(LockedOctrees);
	check(LockedOctrees.Num() == 0);
}

void FVoxelData::BeginGet(const FIntBox& Box, TArray<FVoxelId>& OutOctrees)
{
	OutOctrees.Empty();

	Octree->LockTransactions();
	Octree->BeginGet(Box, OutOctrees, 1e9);

	Algo::Reverse(OutOctrees);
}

bool FVoxelData::TryBeginGet(const FIntBox& Box, int MicroSeconds, TArray<FVoxelId>& OutOctrees)
{
	OutOctrees.Empty();

	Octree->LockTransactions();

	bool bSuccess = Octree->BeginGet(Box, OutOctrees, MicroSeconds);

	Algo::Reverse(OutOctrees);

	if (!bSuccess)
	{
		EndGet(OutOctrees);
	}

	return bSuccess;
}

void FVoxelData::EndGet(TArray<FVoxelId>& LockedOctrees)
{
	Octree->EndGet(LockedOctrees);
	check(LockedOctrees.Num() == 0);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelData::GetValuesAndMaterials(FVoxelValue Values[], FVoxelMaterial Materials[], const FVoxelWorldGeneratorQueryZone& QueryZone, int QueryLOD) const
{
	auto& OctreeBounds = Octree->GetBounds();

	check(OctreeBounds.IsMultipleOf(QueryZone.Step));

	Octree->GetValuesAndMaterials(Values, Materials, QueryZone, QueryLOD);

	if (!OctreeBounds.Contains(QueryZone.Bounds))
	{
		TArray<FIntBox> BoundsOutsideWorld;
		FIntBox::GetRemainingBoxes(QueryZone.Bounds, OctreeBounds, BoundsOutsideWorld);

		for (auto& LocalBounds : BoundsOutsideWorld)
		{
			check(LocalBounds.IsMultipleOf(QueryZone.Step));
			WorldGenerator->GetValuesAndMaterials(Values, Materials, QueryZone.ShrinkTo(LocalBounds), QueryLOD, FVoxelPlaceableItemHolder());
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelData::GetSave(FVoxelUncompressedWorldSave& OutSave)
{
	FVoxelScopeGetLock Lock(this, FIntBox::Infinite);

	OutSave.Init(Depth);
	Octree->Save(OutSave);
	OutSave.Save();
}

void FVoxelData::LoadFromSave(const FVoxelUncompressedWorldSave& Save, TArray<FIntBox>& OutBoundsToUpdate)
{
	FVoxelScopeSetLock Lock(this, FIntBox::Infinite);

	Octree->GetLeavesBounds(OutBoundsToUpdate, 0);
	Octree->ClearData();
	if (bEnableUndoRedo)
	{
		HistoryPosition = 0;
		MaxHistoryPosition = 0;
	}

	int Index = 0;
	Octree->Load(Index, Save, OutBoundsToUpdate);

	check(Index == Save.NumChunks() || Save.GetDepth() > Depth);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

