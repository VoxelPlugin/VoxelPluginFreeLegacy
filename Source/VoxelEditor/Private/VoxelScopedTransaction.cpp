// Copyright 2020 Phyronnaz

#include "VoxelScopedTransaction.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelWorld.h"
#include "Editor.h"

FVoxelUndoRedoChange::FVoxelUndoRedoChange(const TVoxelWeakPtr<FVoxelData>& DataWeakPtr, FName Name, bool bIsUndo)
	: DataWeakPtr(DataWeakPtr)
	, Name(Name)
	, bIsUndo(bIsUndo)
{
}

TUniquePtr<FChange> FVoxelUndoRedoChange::Execute(UObject* Object)
{
	auto* VoxelWorld = Cast<AVoxelWorld>(Object);

	// Check that the world wasn't recreated since
	if (ensure(VoxelWorld) && VoxelWorld->GetDataSharedPtr() == DataWeakPtr.Pin())
	{
		TArray<FIntBox> UpdatedBounds;
		if (bIsUndo)
		{
			ensure(UVoxelBlueprintLibrary::Undo(VoxelWorld, UpdatedBounds));
		}
		else
		{
			ensure(UVoxelBlueprintLibrary::Redo(VoxelWorld, UpdatedBounds));
		}
	}

	return MakeUnique<FVoxelUndoRedoChange>(DataWeakPtr, Name, !bIsUndo);
}

FString FVoxelUndoRedoChange::ToString() const
{
	return FString::Printf(TEXT("Voxel: %s"), *Name.ToString());
}

///////////////////////////////////////////////////////////////////////////////

FVoxelScopedTransaction::FVoxelScopedTransaction(AVoxelWorld* World, FName Name)
	: bValid(ensure(World) && ensure(World->IsCreated()))
{
	if (bValid)
	{
		GEditor->BeginTransaction(TEXT("VoxelEditorTools"), FText::FromName(Name), nullptr);
		if (!ensure(GUndo)) return;
		GUndo->StoreUndo(World, MakeUnique<FVoxelUndoRedoChange>(World->GetDataSharedPtr(), Name, true));
	}
}

FVoxelScopedTransaction::~FVoxelScopedTransaction()
{
	if (bValid)
	{
		GEditor->EndTransaction();
	}
}