// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Misc/Change.h"
#include "VoxelGlobals.h"

class FVoxelData;
class AVoxelWorld;

class FVoxelUndoRedoChange : public ONLY_UE_22_AND_LOWER(FChange) ONLY_UE_23_AND_HIGHER(FSwapChange)
{
public:
	const TVoxelWeakPtr<FVoxelData> DataWeakPtr;
	const FName Name;
	const bool bIsUndo;

	FVoxelUndoRedoChange(const TVoxelWeakPtr<FVoxelData>& DataWeakPtr, FName Name, bool bIsUndo);

	virtual TUniquePtr<FChange> Execute( UObject* Object ) override;
	virtual FString ToString() const override;
};

class FVoxelScopedTransaction
{
public:
	FVoxelScopedTransaction(AVoxelWorld* World, FName Name);
	~FVoxelScopedTransaction();

private:
	const bool bValid;
};