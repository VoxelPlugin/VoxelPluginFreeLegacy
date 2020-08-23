// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Misc/Change.h"
#include "VoxelMinimal.h"

class FVoxelData;
class AVoxelWorld;

enum class EVoxelChangeType
{
	Edit,
	DataSwap
};

class VOXELEDITOR_API FVoxelChangeBase : public FSwapChange
{
public:
	const FName Name;

	explicit FVoxelChangeBase(FName Name);

	virtual FString ToString() const override;
};

class VOXELEDITOR_API FVoxelEditChange : public FVoxelChangeBase
{
public:
	const TVoxelWeakPtr<FVoxelData> DataWeakPtr;
	const bool bIsUndo;

	FVoxelEditChange(const TVoxelWeakPtr<FVoxelData>& DataWeakPtr, FName Name, bool bIsUndo);

	virtual TUniquePtr<FChange> Execute(UObject* Object) override;
};

class VOXELEDITOR_API FVoxelDataSwapChange : public FVoxelChangeBase
{
public:
	const TVoxelSharedRef<FVoxelData> Data;
	
	FVoxelDataSwapChange(const TVoxelSharedRef<FVoxelData>& Data, FName Name);

	virtual TUniquePtr<FChange> Execute(UObject* Object) override;
};

class VOXELEDITOR_API FVoxelScopedTransaction
{
public:
	FVoxelScopedTransaction(AVoxelWorld* World, FName Name, EVoxelChangeType ChangeType);
	~FVoxelScopedTransaction();

private:
	const bool bValid;
};