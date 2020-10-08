// Copyright 2020 Phyronnaz

#include "VoxelScopedTransaction.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelWorld.h"
#include "Editor.h"

FVoxelChangeBase::FVoxelChangeBase(FName Name)
{
}

FString FVoxelChangeBase::ToString() const
{
	return FString::Printf(TEXT("Voxel: %s"), *Name.ToString());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelEditChange::FVoxelEditChange(const TVoxelWeakPtr<FVoxelData>& DataWeakPtr, FName Name, bool bIsUndo)
	: FVoxelChangeBase(Name)
	, DataWeakPtr(DataWeakPtr)
	, bIsUndo(bIsUndo)
{
}

TUniquePtr<FChange> FVoxelEditChange::Execute(UObject* Object)
{
	auto* VoxelWorld = Cast<AVoxelWorld>(Object);

	// Check that the world wasn't recreated since
	if (ensure(VoxelWorld) && VoxelWorld->GetDataSharedPtr() == DataWeakPtr.Pin())
	{
		TArray<FVoxelIntBox> UpdatedBounds;
		if (UpdatedBounds.Num() > 0)
		{
			if (bIsUndo)
			{
				ensure(UVoxelBlueprintLibrary::Undo(VoxelWorld, UpdatedBounds));
			}
			else
			{
				ensure(UVoxelBlueprintLibrary::Redo(VoxelWorld, UpdatedBounds));
			}
		}
	}

	return MakeUnique<FVoxelEditChange>(DataWeakPtr, Name, !bIsUndo);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelDataSwapChange::FVoxelDataSwapChange(const TVoxelSharedRef<FVoxelData>& Data, FName Name)
	: FVoxelChangeBase(Name)
	, Data(Data)
{
}

TUniquePtr<FChange> FVoxelDataSwapChange::Execute(UObject* Object)
{
	auto* VoxelWorld = Cast<AVoxelWorld>(Object);
	if (!ensure(VoxelWorld))
	{
		return nullptr;
	}

	const auto NewData = VoxelWorld->GetDataSharedPtr();

	VoxelWorld->DestroyWorld();
	
	FVoxelWorldCreateInfo Info;
	Info.bOverrideData = true;
	Info.DataOverride_Raw = Data;
	VoxelWorld->CreateWorld(Info);
	
	if (!ensure(NewData.IsValid()))
	{
		return nullptr;
	}
	else
	{
		return MakeUnique<FVoxelDataSwapChange>(NewData.ToSharedRef(), Name);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelScopedTransaction::FVoxelScopedTransaction(AVoxelWorld* World, FName Name, EVoxelChangeType ChangeType)
	: bValid(ensure(World) && ensure(World->IsCreated()))
{
	if (bValid)
	{
		GEditor->BeginTransaction(TEXT("VoxelEditorTools"), FText::FromName(Name), nullptr);
		if (!ensure(GUndo)) return;

		if (ChangeType == EVoxelChangeType::Edit)
		{
			GUndo->StoreUndo(World, MakeUnique<FVoxelEditChange>(World->GetDataSharedPtr(), Name, true));
		}
		else
		{
			check(ChangeType == EVoxelChangeType::DataSwap);
			GUndo->StoreUndo(World, MakeUnique<FVoxelDataSwapChange>(World->GetDataSharedPtr().ToSharedRef(), Name));
		}
	}
}

FVoxelScopedTransaction::~FVoxelScopedTransaction()
{
	if (bValid)
	{
		GEditor->EndTransaction();
	}
}