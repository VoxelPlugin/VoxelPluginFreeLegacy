// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelContainers/VoxelStaticArray.h"
#include "VoxelUtilities/VoxelMiscUtilities.h"

class IVoxelData;
class FVoxelDataOctreeLeaf;
class FVoxelGeneratorInstance;

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel UndoRedo Memory"), STAT_VoxelUndoRedoMemory, STATGROUP_VoxelMemory, VOXEL_API);

enum class EVoxelUndoRedo
{
	Undo,
	Redo
};

class VOXEL_API FVoxelDataOctreeLeafUndoRedo
{
public:
	explicit FVoxelDataOctreeLeafUndoRedo(const FVoxelDataOctreeLeaf& Leaf);
	~FVoxelDataOctreeLeafUndoRedo();

	void ClearFrames(const FVoxelDataOctreeLeaf& Leaf);
	void SaveFrame(const FVoxelDataOctreeLeaf& Leaf, int32 HistoryPosition);

	template<typename T>
	void ClearFramesOfType();

	template<EVoxelUndoRedo Type>
	void UndoRedo(const IVoxelData& Data, FVoxelDataOctreeLeaf& Leaf, int32 HistoryPosition);

public:
	template<EVoxelUndoRedo Type>
	inline bool CanUndoRedo(int32 HistoryPosition) const
	{
		return GetFramesStack<Type>().Num() > 0 && GetFramesStack<Type>().Last()->HistoryPosition == HistoryPosition;
	}
	inline bool IsCurrentFrameEmpty() const
	{
		return CurrentFrame->IsEmpty();
	}
	
	template<EVoxelUndoRedo Type>
	inline auto& GetFramesStack()
	{
		return Type == EVoxelUndoRedo::Undo ? UndoFramesStack : RedoFramesStack;
	}
	template<EVoxelUndoRedo Type>
	inline const auto& GetFramesStack() const
	{
		return Type == EVoxelUndoRedo::Undo ? UndoFramesStack : RedoFramesStack;
	}
	
public:
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
		template<typename TLeaf>
		explicit FFrame(const TLeaf& Leaf)
			: bValuesDirty(Leaf.Values.IsDirty())
			, bMaterialsDirty(Leaf.Materials.IsDirty())
		{
		}
		~FFrame()
		{
			DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelUndoRedoMemory, AllocatedSize);
		}
		
		int32 HistoryPosition = -1;
		
		const bool bValuesDirty;
		const bool bMaterialsDirty;

		TArray<TModifiedValue<FVoxelValue>> Values;
		TArray<TModifiedValue<FVoxelMaterial>> Materials;
		
		mutable uint32 AllocatedSize = 0;
		
		void UpdateStats() const;
		
		inline bool IsEmpty() const
		{
			return Values.Num() == 0 && Materials.Num() == 0;
		}
	};
	struct FAlreadyModified
	{
		TVoxelStaticBitArray<VOXELS_PER_DATA_CHUNK> Values = ForceInit;
		TVoxelStaticBitArray<VOXELS_PER_DATA_CHUNK> Materials = ForceInit;
	};

	FAlreadyModified AlreadyModified;

	TUniquePtr<FFrame> CurrentFrame;
	
	TArray<TUniquePtr<FFrame>> UndoFramesStack;
	TArray<TUniquePtr<FFrame>> RedoFramesStack;
	
	template<EVoxelUndoRedo Type>
	void AddFrameToStack(TUniquePtr<FFrame>& Frame);
};