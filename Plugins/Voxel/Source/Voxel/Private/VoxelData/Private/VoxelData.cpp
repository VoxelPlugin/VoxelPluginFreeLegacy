// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelData.h"
#include "ValueOctree.h"
#include "VoxelSave.h"
#include "GenericPlatformProcess.h"

FVoxelData::FVoxelData(int Depth, AVoxelWorldGenerator* WorldGenerator)
	: Depth(Depth)
	, WorldGenerator(WorldGenerator)
{
	MainOctree = new FValueOctree(WorldGenerator, FIntVector::ZeroValue, Depth, FOctree::GetTopIdFromDepth(Depth));

	GetCount.Reset();

	CanGetEvent = FGenericPlatformProcess::GetSynchEventFromPool(true);
	CanSetEvent = FGenericPlatformProcess::GetSynchEventFromPool(true);

	CanGetEvent->Trigger();
	CanSetEvent->Trigger();
}

FVoxelData::~FVoxelData()
{
	delete MainOctree;
}

int FVoxelData::Size() const
{
	return 16 << Depth;
}

void FVoxelData::BeginSet()
{
	CanGetEvent->Reset();
	CanSetEvent->Wait();
	CanSetEvent->Reset();
}

void FVoxelData::EndSet()
{
	CanSetEvent->Trigger();
	CanGetEvent->Trigger();
}

void FVoxelData::BeginGet()
{
	CanGetEvent->Wait();
	CanSetEvent->Reset();
	GetCount.Increment();
}

void FVoxelData::EndGet()
{
	GetCount.Decrement();
	if (GetCount.GetValue() == 0)
	{
		CanSetEvent->Trigger();
	}
}

void FVoxelData::GetValueAndMaterial(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial) const
{
	ClampToWorld(X, Y, Z);

	MainOctree->GetLeaf(X, Y, Z)->GetValueAndMaterial(X, Y, Z, OutValue, OutMaterial);
}

void FVoxelData::GetValueAndMaterial(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial, FValueOctree*& LastOctree) const
{
	ClampToWorld(X, Y, Z);

	if (UNLIKELY(!LastOctree || !LastOctree->IsLeaf() || !LastOctree->IsInOctree(X, Y, Z)))
	{
		LastOctree = MainOctree->GetLeaf(X, Y, Z);
	}
	LastOctree->GetValueAndMaterial(X, Y, Z, OutValue, OutMaterial);
}

void FVoxelData::SetValue(int X, int Y, int Z, float Value)
{
	check(IsInWorld(X, Y, Z));
	MainOctree->GetLeaf(X, Y, Z)->SetValueAndMaterial(X, Y, Z, Value, FVoxelMaterial(), true, false);
}

void FVoxelData::SetValue(int X, int Y, int Z, float Value, FValueOctree*& LastOctree)
{
	if (UNLIKELY(!LastOctree || !LastOctree->IsLeaf() || !LastOctree->IsInOctree(X, Y, Z)))
	{
		LastOctree = MainOctree->GetLeaf(X, Y, Z);
	}
	LastOctree->SetValueAndMaterial(X, Y, Z, Value, FVoxelMaterial(), true, false);
}

void FVoxelData::SetMaterial(int X, int Y, int Z, FVoxelMaterial Material)
{
	check(IsInWorld(X, Y, Z));
	MainOctree->GetLeaf(X, Y, Z)->SetValueAndMaterial(X, Y, Z, 0, Material, false, true);
}

void FVoxelData::SetMaterial(int X, int Y, int Z, FVoxelMaterial Material, FValueOctree*& LastOctree)
{
	if (UNLIKELY(!LastOctree || !LastOctree->IsLeaf() || !LastOctree->IsInOctree(X, Y, Z)))
	{
		LastOctree = MainOctree->GetLeaf(X, Y, Z);
	}
	LastOctree->SetValueAndMaterial(X, Y, Z, 0, Material, false, true);
}

void FVoxelData::SetValueAndMaterial(int X, int Y, int Z, float Value, FVoxelMaterial Material, FValueOctree*& LastOctree)
{
	if (UNLIKELY(!LastOctree || !LastOctree->IsLeaf() || !LastOctree->IsInOctree(X, Y, Z)))
	{
		LastOctree = MainOctree->GetLeaf(X, Y, Z);
	}
	LastOctree->SetValueAndMaterial(X, Y, Z, Value, Material, true, true);
}

bool FVoxelData::IsInWorld(int X, int Y, int Z) const
{
	int S = Size() / 2;
	return -S <= X && X < S
		&& -S <= Y && Y < S
		&& -S <= Z && Z < S;
}

FORCEINLINE void FVoxelData::ClampToWorld(int& X, int& Y, int& Z) const
{
	int S = Size() / 2;
	X = FMath::Clamp(X, -S, S - 1);
	Y = FMath::Clamp(Y, -S, S - 1);
	Z = FMath::Clamp(Z, -S, S - 1);
}

void FVoxelData::GetSave(FVoxelWorldSave& OutSave) const
{
	std::list<TSharedRef<FVoxelChunkSave>> SaveList;
	MainOctree->AddDirtyChunksToSaveList(SaveList);
	OutSave.Init(Depth, SaveList);
}

void FVoxelData::LoadFromSaveAndGetModifiedPositions(FVoxelWorldSave& Save, std::forward_list<FIntVector>& OutModifiedPositions, bool bReset)
{
	if (bReset)
	{
		MainOctree->GetDirtyChunksPositions(OutModifiedPositions);
		delete MainOctree;
		MainOctree = new FValueOctree(WorldGenerator, FIntVector::ZeroValue, Depth, FOctree::GetTopIdFromDepth(Depth));
	}

	auto SaveList = Save.GetChunksList();
	MainOctree->LoadFromSaveAndGetModifiedPositions(SaveList, OutModifiedPositions);
	check(SaveList.empty());
}

void FVoxelData::GetDiffArrays(std::forward_list<TArray<FVoxelValueDiff>>& OutValueDiffPacketsList, std::forward_list<TArray<FVoxelMaterialDiff>>& OutColorDiffPacketsList) const
{
	VoxelValueDiffArray ValueDiffArray;
	VoxelMaterialDiffArray ColorDiffArray;

	MainOctree->AddChunksToDiffArrays(ValueDiffArray, ColorDiffArray);

	ValueDiffArray.AddPackets(OutValueDiffPacketsList);
	ColorDiffArray.AddPackets(OutColorDiffPacketsList);
}

void FVoxelData::LoadFromDiffArrayAndGetModifiedPositions(TArray<FVoxelValueDiff>& ValueDiffArray, TArray<FVoxelMaterialDiff>& ColorDiffArray, std::forward_list<FIntVector>& OutModifiedPositions)
{
	std::forward_list<FVoxelValueDiff> ValueDiffList;
	std::forward_list<FVoxelMaterialDiff> ColorDiffList;

	for (int i = ValueDiffArray.Num() - 1; i >= 0; i--)
	{
		ValueDiffList.push_front(ValueDiffArray[i]);
	}
	for (int i = ColorDiffArray.Num() - 1; i >= 0; i--)
	{
		ColorDiffList.push_front(ColorDiffArray[i]);
	}

	MainOctree->LoadFromDiffListAndGetModifiedPositions(ValueDiffList, ColorDiffList, OutModifiedPositions);
}
