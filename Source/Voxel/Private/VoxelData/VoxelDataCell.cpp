// Copyright 2020 Phyronnaz

#include "VoxelData/VoxelDataCell.h"

DEFINE_STAT(STAT_VoxelUndoRedoMemory);
DEFINE_STAT(STAT_VoxelMultiplayerMemory);

void FVoxelDataCellUndoRedo::ClearFrames()
{
	CurrentFrame = MakeUnique<FFrame>();
	UndoFramesStack.Empty();
	RedoFramesStack.Empty();
}

void FVoxelDataCellUndoRedo::SaveFrame(int32 HistoryPosition)
{
	VOXEL_SLOW_FUNCTION_COUNTER();
	
	if (!CurrentFrame->IsEmpty())
	{
		CurrentFrame->HistoryPosition = HistoryPosition;
		AddFrameToStack<EStackType::Undo>(CurrentFrame);
		check(!CurrentFrame);

		CurrentFrame = MakeUnique<FFrame>();
		AlreadyModified.Values.Clear();
		AlreadyModified.Materials.Clear();
		AlreadyModified.Foliage.Clear();
	}
	if (RedoFramesStack.Num() > 0)
	{
		RedoFramesStack.Empty();
	}
}

void FVoxelDataCellUndoRedo::Undo(FVoxelValue* Values, FVoxelMaterial* Materials, FVoxelFoliage* Foliage, int32 HistoryPosition)
{
	check(CurrentFrame->IsEmpty());
	if (!ensure(CanUndo(HistoryPosition))) return;

	const TUniquePtr<const FFrame> UndoFrame = UndoFramesStack.Pop(false);
	TUniquePtr<FFrame> RedoFrame = MakeUnique<FFrame>();
	RedoFrame->HistoryPosition = HistoryPosition + 1;

	check(!UndoFrame->IsEmpty());

	const auto Apply = [](const auto& UndoData, auto& RedoData, auto* RESTRICT Data)
	{
		RedoData.SetNumUninitialized(UndoData.Num());
		for (int32 I = UndoData.Num() - 1; I >= 0; I--) // Reversed because we are modifying Data array
		{
			checkVoxelSlow(Data);
			checkVoxelSlow(UndoData.IsValidIndex(I));
			checkVoxelSlow(RedoData.IsValidIndex(I));
			const auto ModifiedValue = UndoData.GetData()[I];
			RedoData.GetData()[I] = decltype(ModifiedValue)(ModifiedValue.Index, Data[ModifiedValue.Index]);
			Data[ModifiedValue.Index] = ModifiedValue.Value;
		}
	};

	Apply(UndoFrame->Values, RedoFrame->Values, Values);
	Apply(UndoFrame->Materials, RedoFrame->Materials, Materials);
	Apply(UndoFrame->Foliage, RedoFrame->Foliage, Foliage);

	AddFrameToStack<EStackType::Redo>(RedoFrame);
}

void FVoxelDataCellUndoRedo::Redo(FVoxelValue* Values, FVoxelMaterial* Materials, FVoxelFoliage* Foliage, int32 HistoryPosition)
{
	check(CurrentFrame->IsEmpty());
	if (!ensure(CanRedo(HistoryPosition))) return;

	const TUniquePtr<const FFrame> RedoFrame = RedoFramesStack.Pop(false);
	TUniquePtr<FFrame> UndoFrame = MakeUnique<FFrame>();
	UndoFrame->HistoryPosition = HistoryPosition - 1;

	check(!RedoFrame->IsEmpty());

	const auto Apply = [](auto& UndoData, const auto& RedoData, auto* RESTRICT Data)
	{
		UndoData.SetNumUninitialized(RedoData.Num());
		for (int32 I = RedoData.Num() - 1; I >= 0; I--) // Reversed because we are modifying Data array
		{
			checkVoxelSlow(Data);
			checkVoxelSlow(UndoData.IsValidIndex(I));
			checkVoxelSlow(RedoData.IsValidIndex(I));
			const auto ModifiedValue = RedoData.GetData()[I];
			UndoData.GetData()[I] = decltype(ModifiedValue)(ModifiedValue.Index, Data[ModifiedValue.Index]);
			Data[ModifiedValue.Index] = ModifiedValue.Value;
		}
	};

	Apply(UndoFrame->Values, RedoFrame->Values, Values);
	Apply(UndoFrame->Materials, RedoFrame->Materials, Materials);
	Apply(UndoFrame->Foliage, RedoFrame->Foliage, Foliage);

	AddFrameToStack<EStackType::Undo>(UndoFrame);
}

template<FVoxelDataCellUndoRedo::EStackType Type>
void FVoxelDataCellUndoRedo::AddFrameToStack(TUniquePtr<FFrame>& Frame)
{
	{
		VOXEL_SLOW_SCOPE_COUNTER("Shrink");
		Frame->Values.Shrink();
		Frame->Materials.Shrink();
		Frame->Foliage.Shrink();
	}
	INC_MEMORY_STAT_BY(STAT_VoxelUndoRedoMemory, Frame->GetAllocatedSize());
	auto& Stack = Type == EStackType::Undo ? UndoFramesStack : RedoFramesStack;
	Stack.Add(MoveTemp(Frame));
	check(!Frame);
}