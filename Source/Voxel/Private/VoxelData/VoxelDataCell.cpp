// Copyright 2019 Phyronnaz

#include "VoxelData/VoxelDataCell.h"

void FVoxelDataCellUndoRedo::ClearFrames()
{
	CurrentFrame = MakeUnique<Frame>();
	UndoFramesStack.Empty();
	RedoFramesStack.Empty();
}

void FVoxelDataCellUndoRedo::SaveFrame(int HistoryPosition)
{
	if (!CurrentFrame->IsEmpty())
	{
		CurrentFrame->HistoryPosition = HistoryPosition;
		AddFrameToStack<EStackType::Undo>(CurrentFrame);
		check(!CurrentFrame);

		CurrentFrame = MakeUnique<Frame>();
		AlreadyModifiedValues.Empty();
		AlreadyModifiedMaterials.Empty();
	}
	if (RedoFramesStack.Num() > 0)
	{
		RedoFramesStack.Empty();
	}
}

bool FVoxelDataCellUndoRedo::TryUndo(FVoxelDataCell* Cell, int HistoryPosition)
{
	check(CurrentFrame->IsEmpty());

	if (UndoFramesStack.Num() > 0 && UndoFramesStack.Last()->HistoryPosition == HistoryPosition)
	{
		auto* Values    = Cell->GetArray<FVoxelValue>();
		auto* Materials = Cell->GetArray<FVoxelMaterial>();

		TUniquePtr<Frame> UndoFrame = UndoFramesStack.Pop();
		TUniquePtr<Frame> RedoFrame = MakeUnique<Frame>();
		RedoFrame->HistoryPosition = HistoryPosition + 1;

		check(!UndoFrame->IsEmpty());

		RedoFrame->ModifiedValues.SetNumUninitialized(UndoFrame->ModifiedValues.Num());
		for (int I = UndoFrame->ModifiedValues.Num() - 1; I >= 0; I--) // Reversed because we are modifying Values array
		{
			auto& M = UndoFrame->ModifiedValues[I];
			RedoFrame->ModifiedValues[I] = TModifiedValue<FVoxelValue>(M.Index, Values[M.Index]);
			Values[M.Index] = M.Value;
		}

		RedoFrame->ModifiedMaterials.SetNumUninitialized(UndoFrame->ModifiedMaterials.Num());
		for (int I = UndoFrame->ModifiedMaterials.Num() - 1; I >= 0; I--) // Reversed because we are modifying Materials array
		{
			auto& M = UndoFrame->ModifiedMaterials[I];
			RedoFrame->ModifiedMaterials[I] = TModifiedValue<FVoxelMaterial>(M.Index, Materials[M.Index]);
			Materials[M.Index] = M.Value;
		}

		AddFrameToStack<EStackType::Redo>(RedoFrame);

		return true;
	}
	else
	{
		return false;
	}
}

bool FVoxelDataCellUndoRedo::TryRedo(FVoxelDataCell* Cell, int HistoryPosition)
{
	check(CurrentFrame->IsEmpty());

	if (RedoFramesStack.Num() > 0 && RedoFramesStack.Last()->HistoryPosition == HistoryPosition)
	{
		auto* Values    = Cell->GetArray<FVoxelValue>();
		auto* Materials = Cell->GetArray<FVoxelMaterial>();

		TUniquePtr<Frame> RedoFrame = RedoFramesStack.Pop();
		TUniquePtr<Frame> UndoFrame = MakeUnique<Frame>();
		UndoFrame->HistoryPosition = HistoryPosition - 1;

		check(!RedoFrame->IsEmpty());

		UndoFrame->ModifiedValues.SetNumUninitialized(RedoFrame->ModifiedValues.Num());
		for (int I = RedoFrame->ModifiedValues.Num() - 1; I >= 0; I--) // Reversed because we are modifying Values array
		{
			auto& M = RedoFrame->ModifiedValues[I];
			UndoFrame->ModifiedValues[I] = TModifiedValue<FVoxelValue>(M.Index, Values[M.Index]);
			Values[M.Index] = M.Value;
		}

		UndoFrame->ModifiedMaterials.SetNumUninitialized(RedoFrame->ModifiedMaterials.Num());
		for (int I = RedoFrame->ModifiedMaterials.Num() - 1; I >= 0; I--) // Reversed because we are modifying Materials array
		{
			auto& M = RedoFrame->ModifiedMaterials[I];
			UndoFrame->ModifiedMaterials[I] = TModifiedValue<FVoxelMaterial>(M.Index, Materials[M.Index]);
			Materials[M.Index] = M.Value;
		}

		AddFrameToStack<EStackType::Undo>(UndoFrame);

		return true;
	}
	else
	{
		return false;
	}
}