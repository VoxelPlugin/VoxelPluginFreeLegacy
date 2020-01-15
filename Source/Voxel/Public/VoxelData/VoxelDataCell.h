// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelDiff.h"
#include "StackArray.h"
#include "VoxelMiscUtilities.h"

// TODO split file into undo and multiplayer

DECLARE_MEMORY_STAT_EXTERN(TEXT("Voxel UndoRedo Memory"), STAT_VoxelUndoRedoMemory, STATGROUP_VoxelMemory, VOXEL_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Voxel Multiplayer Memory"), STAT_VoxelMultiplayerMemory, STATGROUP_VoxelMemory, VOXEL_API);

class FVoxelDataCellUndoRedo
{
public:
	FVoxelDataCellUndoRedo()
	{
		INC_MEMORY_STAT_BY(STAT_VoxelUndoRedoMemory, sizeof(FVoxelDataCellUndoRedo));
	}
	~FVoxelDataCellUndoRedo()
	{
		DEC_MEMORY_STAT_BY(STAT_VoxelUndoRedoMemory, sizeof(FVoxelDataCellUndoRedo));
	}
	
	void ClearFrames();
	void SaveFrame(int32 HistoryPosition);

	inline bool CanUndo(int32 HistoryPosition) const
	{
		return UndoFramesStack.Num() > 0 && UndoFramesStack.Last()->HistoryPosition == HistoryPosition;
	}
	inline bool CanRedo(int32 HistoryPosition) const
	{
		return RedoFramesStack.Num() > 0 && RedoFramesStack.Last()->HistoryPosition == HistoryPosition;
	}

	void Undo(FVoxelValue* Values, FVoxelMaterial* Materials, FVoxelFoliage* Foliage, int32 HistoryPosition);
	void Redo(FVoxelValue* Values, FVoxelMaterial* Materials, FVoxelFoliage* Foliage, int32 HistoryPosition);

	inline bool IsCurrentFrameEmpty() const
	{
		return CurrentFrame->IsEmpty();
	}
	inline const auto& GetUndoFramesStack() const
	{
		return UndoFramesStack;
	}
	
	template<typename T>
	FORCEINLINE void SavePreviousValue(FVoxelCellIndex Index, T Value)
	{
		auto& AlreadyModifiedT = FVoxelUtilities::TValuesMaterialsSelector<T>::Get(AlreadyModified);
		if (!AlreadyModifiedT.Test(Index))
		{
			AlreadyModifiedT.Set(Index);
			FVoxelUtilities::TValuesMaterialsSelector<T>::Get(*CurrentFrame).Emplace(Index, Value);
		}
	}

private:
	template<typename T>
	struct TModifiedValue
	{
		FVoxelCellIndex Index;
		T Value;

		TModifiedValue(FVoxelCellIndex Index, T Value) : Index(Index), Value(Value) {}
	};
	struct FFrame
	{
		FFrame() = default;
		~FFrame()
		{
			DEC_MEMORY_STAT_BY(STAT_VoxelUndoRedoMemory, GetAllocatedSize());
		}

		int32 HistoryPosition = -1;
		TArray<TModifiedValue<FVoxelValue   >> Values;
		TArray<TModifiedValue<FVoxelMaterial>> Materials;
		TArray<TModifiedValue<FVoxelFoliage>> Foliage;

		inline int32 GetAllocatedSize() const
		{
			return sizeof(FFrame) + Values.GetAllocatedSize() + Materials.GetAllocatedSize() + Foliage.GetAllocatedSize();
		}
		inline bool IsEmpty() const
		{
			return Values.Num() == 0 && Materials.Num() == 0 && Foliage.Num() == 0;
		}
	};
	struct FAlreadyModified
	{
		TStackBitArray<VOXELS_PER_DATA_CHUNK> Values = ForceInit;
		TStackBitArray<VOXELS_PER_DATA_CHUNK> Materials = ForceInit;
		TStackBitArray<VOXELS_PER_DATA_CHUNK> Foliage = ForceInit;
	};

	FAlreadyModified AlreadyModified;

	TUniquePtr<FFrame> CurrentFrame = MakeUnique<FFrame>();
	
	TArray<TUniquePtr<FFrame>> UndoFramesStack;
	TArray<TUniquePtr<FFrame>> RedoFramesStack;

	enum class EStackType
	{
		Undo,
		Redo
	};
	
	template<EStackType Type>
	void AddFrameToStack(TUniquePtr<FFrame>& Frame);
};

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

class FVoxelDataCellMultiplayer
{
public:
	struct FDirty
	{
		// TODO bit arrays would be better
		TSet<FVoxelCellIndex> Values;
		TSet<FVoxelCellIndex> Materials;
		TEmptyArray<FVoxelCellIndex> Foliage;
	};
	FDirty Dirty;

public:
	template<typename T>
	FORCEINLINE void MarkIndexDirty(FVoxelCellIndex Index)
	{
		FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Dirty).Add(Index);
	}
	
	template<typename T>
	void AddToDiffQueueAndReset(T* Data, TArray<TVoxelDiff<T>>& OutDiffQueue)
	{
		auto& DirtyT = FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Dirty);
		for (int32 Index : DirtyT)
		{
			OutDiffQueue.Emplace(Index, Data[Index]);
		}
		DirtyT.Empty();
	}

	template<typename T>
	bool IsNetworkDirty()
	{
		return FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Dirty).Num() > 0;
	}
};