// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelDiff.h"
#include "VoxelUtilities/VoxelMiscUtilities.h"

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Multiplayer Memory"), STAT_VoxelMultiplayerMemory, STATGROUP_VoxelMemory, VOXEL_API);

template<typename T>
struct TEmptyArray
{
	inline int32 Add(T Value)
	{
		return -1;
	}
	inline int32 Num() const
	{
		return 0;
	}
	inline T* begin() const
	{
		return nullptr;
	}
	inline T* end() const
	{
		return nullptr;
	}
};

class FVoxelDataOctreeLeafMultiplayer
{
public:
	struct FDirty
	{
		// TODO bit arrays would be better
		TSet<FVoxelCellIndex> Values;
		TSet<FVoxelCellIndex> Materials;
	};
	FDirty Dirty;

public:
	template<typename T>
	FORCEINLINE void MarkIndexDirty(FVoxelCellIndex Index)
	{
		FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Dirty).Add(Index);
	}
	
	template<typename T, typename TData>
	void AddToDiffQueueAndReset(const TData& Data, TArray<TVoxelDiff<T>>& OutDiffQueue)
	{
		auto& DirtyT = FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Dirty);
		for (int32 Index : DirtyT)
		{
			OutDiffQueue.Emplace(Index, Data.Get(Index));
		}
		DirtyT.Empty();
	}

	template<typename T>
	bool IsNetworkDirty()
	{
		return FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Dirty).Num() > 0;
	}
};