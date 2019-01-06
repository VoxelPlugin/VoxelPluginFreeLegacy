// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelLogStatDefinitions.h"
#include "IntBox.h"
#include "VoxelDiff.h"

DECLARE_MEMORY_STAT(TEXT("Voxel UndoRedo Memory"), STAT_VoxelUndoRedoMemory, STATGROUP_VoxelMemory);
DECLARE_MEMORY_STAT(TEXT("Voxel Cells Memory"), STAT_VoxelCellsMemory, STATGROUP_VoxelMemory);

namespace FVoxelDataCellUtilities
{
	inline FVoxelCellIndex IndexFromCoordinates(int X, int Y, int Z)
	{
		checkVoxelSlow(0 <= X && X < VOXEL_CELL_SIZE && 0 <= Y && Y < VOXEL_CELL_SIZE && 0 <= Z && Z < VOXEL_CELL_SIZE);
		return X + VOXEL_CELL_SIZE * Y + VOXEL_CELL_SIZE * VOXEL_CELL_SIZE * Z;
	}
	inline void CoordinatesFromIndex(FVoxelCellIndex Index, int& OutX, int& OutY, int& OutZ)
	{
		checkVoxelSlow(0 <= Index && Index < VOXEL_CELL_COUNT);
		OutZ = Index / (VOXEL_CELL_SIZE * VOXEL_CELL_SIZE);
		Index -= OutZ;
		OutY = Index / VOXEL_CELL_SIZE;
		Index -= OutY;
		OutX = Index;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelDataCell
{
public:
	FVoxelDataCell()
	{
		INC_MEMORY_STAT_BY(STAT_VoxelCellsMemory, sizeof(FVoxelDataCell));
	}
	~FVoxelDataCell()
	{
		DEC_MEMORY_STAT_BY(STAT_VoxelCellsMemory, sizeof(FVoxelDataCell));
	}

	template<typename T> inline       T* GetArray();
	template<typename T> inline const T* GetArray() const;

	template<typename T> inline       TVoxelBuffer<T>& GetBuffer();
	template<typename T> inline const TVoxelBuffer<T>& GetBuffer() const;

	template<typename T> inline void SetBufferAsDirty();

	template<typename T> inline bool IsBufferDirty() const;

private:
	TVoxelBuffer<FVoxelValue> Values;
	TVoxelBuffer<FVoxelMaterial> Materials;
	bool bValuesAreDirty = false;
	bool bMaterialsAreDirty = false;
};

template<> inline       FVoxelValue*    FVoxelDataCell::GetArray<FVoxelValue   >()       { return Values   .data(); }
template<> inline const FVoxelValue*    FVoxelDataCell::GetArray<FVoxelValue   >() const { return Values   .data(); }
template<> inline       FVoxelMaterial* FVoxelDataCell::GetArray<FVoxelMaterial>()       { return Materials.data(); }
template<> inline const FVoxelMaterial* FVoxelDataCell::GetArray<FVoxelMaterial>() const { return Materials.data(); }

template<> inline       TVoxelBuffer<FVoxelValue>&    FVoxelDataCell::GetBuffer<FVoxelValue   >()       { return Values   ; }
template<> inline const TVoxelBuffer<FVoxelValue>&    FVoxelDataCell::GetBuffer<FVoxelValue   >() const { return Values   ; }
template<> inline       TVoxelBuffer<FVoxelMaterial>& FVoxelDataCell::GetBuffer<FVoxelMaterial>()       { return Materials; }
template<> inline const TVoxelBuffer<FVoxelMaterial>& FVoxelDataCell::GetBuffer<FVoxelMaterial>() const { return Materials; }

template<> inline void FVoxelDataCell::SetBufferAsDirty<FVoxelValue   >() { bValuesAreDirty    = true; }
template<> inline void FVoxelDataCell::SetBufferAsDirty<FVoxelMaterial>() { bMaterialsAreDirty = true; }

template<> inline bool FVoxelDataCell::IsBufferDirty<FVoxelValue   >() const { return bValuesAreDirty   ; }
template<> inline bool FVoxelDataCell::IsBufferDirty<FVoxelMaterial>() const { return bMaterialsAreDirty; }


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelDataCellUndoRedo
{
public:
	template<typename T>
	struct TModifiedValue
	{
		FVoxelCellIndex Index;
		T Value;

		TModifiedValue(FVoxelCellIndex Index, T Value) : Index(Index), Value(Value) {}
	};

	struct Frame
	{
		~Frame()
		{
			DEC_MEMORY_STAT_BY(STAT_VoxelUndoRedoMemory, GetAllocatedSize());
		}

		int HistoryPosition = -1;
		TArray<TModifiedValue<FVoxelValue   >> ModifiedValues;
		TArray<TModifiedValue<FVoxelMaterial>> ModifiedMaterials;

		template<typename T> inline TArray<TModifiedValue<T>>& GetModified();

		inline int GetAllocatedSize() const { return sizeof(Frame) + ModifiedValues.GetAllocatedSize() + ModifiedMaterials.GetAllocatedSize(); }
		inline bool IsEmpty() const { return ModifiedValues.Num() == 0 && ModifiedMaterials.Num() == 0; }
	};

public:
	void ClearFrames();
	void SaveFrame(int HistoryPosition);
	bool TryUndo(FVoxelDataCell* Cell, int HistoryPosition);
	bool TryRedo(FVoxelDataCell* Cell,int HistoryPosition);
	bool IsCurrentFrameEmpty() const { return CurrentFrame->IsEmpty(); }
	
	template<typename T>
	void AddEdit(FVoxelCellIndex Index, T& Value)
	{
		auto& AlreadyModified = GetAlreadyModified<T>();
		if (!AlreadyModified.Contains(Index))
		{
			AlreadyModified.Add(Index);
			CurrentFrame->GetModified<T>().Emplace(Index, Value);
		}
	}

private:
	TUniquePtr<Frame> CurrentFrame = MakeUnique<Frame>();
	TSet<FVoxelCellIndex> AlreadyModifiedValues;
	TSet<FVoxelCellIndex> AlreadyModifiedMaterials;

	TArray<TUniquePtr<Frame>> UndoFramesStack;
	TArray<TUniquePtr<Frame>> RedoFramesStack;

	enum class EStackType { Undo, Redo };
	template<EStackType Type>
	void AddFrameToStack(TUniquePtr<Frame>& Frame)
	{
		INC_MEMORY_STAT_BY(STAT_VoxelUndoRedoMemory, Frame->GetAllocatedSize());

		auto& Stack = Type == EStackType::Undo ? UndoFramesStack : RedoFramesStack;
		Stack.Add(MoveTemp(Frame));

		check(!Frame);
	}

	template<typename T> inline TSet<FVoxelCellIndex>& GetAlreadyModified();
};

template<> inline TArray<FVoxelDataCellUndoRedo::TModifiedValue<FVoxelValue   >>& FVoxelDataCellUndoRedo::Frame::GetModified<FVoxelValue   >() { return ModifiedValues;    }
template<> inline TArray<FVoxelDataCellUndoRedo::TModifiedValue<FVoxelMaterial>>& FVoxelDataCellUndoRedo::Frame::GetModified<FVoxelMaterial>() { return ModifiedMaterials; }

template<> inline TSet<FVoxelCellIndex>& FVoxelDataCellUndoRedo::GetAlreadyModified<FVoxelValue   >() { return AlreadyModifiedValues   ; }
template<> inline TSet<FVoxelCellIndex>& FVoxelDataCellUndoRedo::GetAlreadyModified<FVoxelMaterial>() { return AlreadyModifiedMaterials; }