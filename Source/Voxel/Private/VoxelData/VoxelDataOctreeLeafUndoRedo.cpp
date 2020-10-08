// Copyright 2020 Phyronnaz

#include "VoxelData/VoxelDataOctreeLeafUndoRedo.h"
#include "VoxelData/VoxelDataOctree.h"

FVoxelDataOctreeLeafUndoRedo::FVoxelDataOctreeLeafUndoRedo(const FVoxelDataOctreeLeaf& Leaf)
	: CurrentFrame(MakeUnique<FFrame>(Leaf))
{
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelUndoRedoMemory, sizeof(FVoxelDataOctreeLeafUndoRedo));
}

FVoxelDataOctreeLeafUndoRedo::~FVoxelDataOctreeLeafUndoRedo()
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelUndoRedoMemory, sizeof(FVoxelDataOctreeLeafUndoRedo));
}

void FVoxelDataOctreeLeafUndoRedo::ClearFrames(const FVoxelDataOctreeLeaf& Leaf)
{
	CurrentFrame = MakeUnique<FFrame>(Leaf);
	UndoFramesStack.Empty();
	RedoFramesStack.Empty();
}

void FVoxelDataOctreeLeafUndoRedo::SaveFrame(const FVoxelDataOctreeLeaf& Leaf, int32 HistoryPosition)
{
	VOXEL_SLOW_FUNCTION_COUNTER();
	
	if (!CurrentFrame->IsEmpty())
	{
		CurrentFrame->HistoryPosition = HistoryPosition;
		AddFrameToStack<EVoxelUndoRedo::Undo>(CurrentFrame);
		check(!CurrentFrame);

		CurrentFrame = MakeUnique<FFrame>(Leaf);

		AlreadyModified.Values.Clear();
		AlreadyModified.Materials.Clear();
	}
	if (RedoFramesStack.Num() > 0)
	{
		RedoFramesStack.Empty();
	}
}

template<typename T>
void FVoxelDataOctreeLeafUndoRedo::ClearFramesOfType()
{
	const auto ClearFrame = [](FFrame& Frame)
	{
		FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Frame).Empty();
	};
	
	ClearFrame(*CurrentFrame);
	for (auto& Frame : UndoFramesStack)
	{
		ClearFrame(*Frame);
	}
	for (auto& Frame : RedoFramesStack)
	{
		ClearFrame(*Frame);
	}
}

template VOXEL_API void FVoxelDataOctreeLeafUndoRedo::ClearFramesOfType<FVoxelValue>();
template VOXEL_API void FVoxelDataOctreeLeafUndoRedo::ClearFramesOfType<FVoxelMaterial>();

template<EVoxelUndoRedo Type>
void FVoxelDataOctreeLeafUndoRedo::UndoRedo(const IVoxelData& Data, FVoxelDataOctreeLeaf& Leaf, int32 HistoryPosition)
{
	check(CurrentFrame->IsEmpty());
	check(CanUndoRedo<Type>(HistoryPosition));

	const TUniquePtr<const FFrame> Frame = GetFramesStack<Type>().Pop(false);
	check(Frame->HistoryPosition == HistoryPosition);
	
	TUniquePtr<FFrame> NewFrame = MakeUnique<FFrame>(Leaf);
	// If Type is Undo NewFrame is a redo frame, so + 1. Else it's an undo frame so -1
	NewFrame->HistoryPosition = HistoryPosition + (Type == EVoxelUndoRedo::Undo ? 1 : -1);

	check(!Frame->IsEmpty());

	const auto Apply = [&](auto TypeInst)
	{
		using T = decltype(TypeInst);

		const TArray<TModifiedValue<T>>& FrameData = FVoxelUtilities::TValuesMaterialsSelector<T>::Get(*Frame);
		TArray<TModifiedValue<T>>& NewFrameData = FVoxelUtilities::TValuesMaterialsSelector<T>::Get(*NewFrame);
		TVoxelDataOctreeLeafData<T>& DataHolder = FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Leaf);
		
		if (FrameData.Num() == 0) return;

		if (!DataHolder.HasData())
		{
			// Data was reverted to generator value
			
			DataHolder.CreateData(Data, [&](T* RESTRICT DataPtr)
			{
				TVoxelQueryZone<T> QueryZone(Leaf.GetBounds(), DataPtr);
				Leaf.GetFromGeneratorAndAssets(*Data.Generator, QueryZone, 0);
			});
		}
		DataHolder.PrepareForWrite(Data);

		NewFrameData.SetNumUninitialized(FrameData.Num());

		const TModifiedValue<T>* RESTRICT const FrameDataPtr = FrameData.GetData();
		TModifiedValue<T>* RESTRICT const NewFrameDataPtr = NewFrameData.GetData();

		checkVoxelSlow(FrameDataPtr);
		checkVoxelSlow(NewFrameDataPtr);
		
		for (int32 Index = 0; Index < FrameData.Num(); Index++)
		{
			checkVoxelSlow(FrameData.IsValidIndex(Index));
			checkVoxelSlow(NewFrameData.IsValidIndex(Index));

			const auto ModifiedValue = FrameDataPtr[Index];
			auto& ValueRef = DataHolder.GetRef(ModifiedValue.Index);
			
			NewFrameDataPtr[Index] = TModifiedValue<T>(ModifiedValue.Index, ValueRef);
			ValueRef = ModifiedValue.Value;
		}

		if (TIsSame<T, FVoxelValue>::Value) DataHolder.SetIsDirty(Frame->bValuesDirty, Data);
		if (TIsSame<T, FVoxelMaterial>::Value) DataHolder.SetIsDirty(Frame->bMaterialsDirty, Data);
	};

	Apply(FVoxelValue());
	Apply(FVoxelMaterial());

	AddFrameToStack<Type == EVoxelUndoRedo::Undo ? EVoxelUndoRedo::Redo : EVoxelUndoRedo::Undo>(NewFrame);
}

template VOXEL_API void FVoxelDataOctreeLeafUndoRedo::UndoRedo<EVoxelUndoRedo::Undo>(const IVoxelData&, FVoxelDataOctreeLeaf&, int32);
template VOXEL_API void FVoxelDataOctreeLeafUndoRedo::UndoRedo<EVoxelUndoRedo::Redo>(const IVoxelData&, FVoxelDataOctreeLeaf&, int32);

void FVoxelDataOctreeLeafUndoRedo::FFrame::UpdateStats() const
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelUndoRedoMemory, AllocatedSize);
	AllocatedSize = sizeof(FFrame) + Values.GetAllocatedSize() + Materials.GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelUndoRedoMemory, AllocatedSize);
}

template<EVoxelUndoRedo Type>
void FVoxelDataOctreeLeafUndoRedo::AddFrameToStack(TUniquePtr<FFrame>& Frame)
{
	{
		VOXEL_SLOW_SCOPE_COUNTER("Shrink");
		Frame->Values.Shrink();
		Frame->Materials.Shrink();
	}

	Frame->UpdateStats();
	
	GetFramesStack<Type>().Add(MoveTemp(Frame));
	check(!Frame);
}
