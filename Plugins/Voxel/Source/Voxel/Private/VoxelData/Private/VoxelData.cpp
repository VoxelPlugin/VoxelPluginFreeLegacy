// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelData.h"
#include "ValueOctree.h"
#include "VoxelSave.h"

VoxelData::VoxelData(int Depth, AVoxelWorldGenerator* WorldGenerator)
	: Depth(Depth)
	, WorldGenerator(WorldGenerator)
{
	MainOctree = new ValueOctree(WorldGenerator, FIntVector::ZeroValue, Depth, Octree::GetTopIdFromDepth(Depth));
}

VoxelData::~VoxelData()
{
	delete MainOctree;
}

int VoxelData::Size() const
{
	return 16 << Depth;
}

float VoxelData::GetValue(int X, int Y, int Z) const
{
	check(IsInWorld(X, Y, Z));

	FVoxelMaterial Material;
	float Value;
	GetValueAndMaterial(X, Y, Z, Value, Material);
	return Value;
}

FVoxelMaterial VoxelData::GetMaterial(int X, int Y, int Z) const
{
	check(IsInWorld(X, Y, Z));

	FVoxelMaterial Material;
	float Value;
	GetValueAndMaterial(X, Y, Z, Value, Material);
	return Material;
}

void VoxelData::GetValueAndMaterial(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial) const
{
	ClampToWorld(X, Y, Z);

	MainOctree->GetLeaf(X, Y, Z)->GetValueAndMaterial(X, Y, Z, OutValue, OutMaterial);
}

void VoxelData::GetValueAndMaterial(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial, ValueOctree*& LastOctree) const
{
	ClampToWorld(X, Y, Z);

	if (UNLIKELY(!LastOctree || !LastOctree->IsLeaf() || !LastOctree->IsInOctree(X, Y, Z)))
	{
		LastOctree = MainOctree->GetLeaf(X, Y, Z);
	}
	LastOctree->GetValueAndMaterial(X, Y, Z, OutValue, OutMaterial);
}

void VoxelData::SetValue(int X, int Y, int Z, float Value)
{
	check(IsInWorld(X, Y, Z));
	MainOctree->GetLeaf(X, Y, Z)->SetValue(X, Y, Z, Value);
}

void VoxelData::SetMaterial(int X, int Y, int Z, FVoxelMaterial Material)
{
	check(IsInWorld(X, Y, Z));
	MainOctree->GetLeaf(X, Y, Z)->SetMaterial(X, Y, Z, Material);
}

bool VoxelData::IsInWorld(int X, int Y, int Z) const
{
	int S = Size() / 2;
	return -S <= X && X < S
		&& -S <= Y && Y < S
		&& -S <= Z && Z < S;
}

FORCEINLINE void VoxelData::ClampToWorld(int& X, int& Y, int& Z) const
{
	int S = Size() / 2;
	X = FMath::Clamp(X, -S, S - 1);
	Y = FMath::Clamp(Y, -S, S - 1);
	Z = FMath::Clamp(Z, -S, S - 1);
}

FVoxelWorldSave VoxelData::GetSave() const
{
	std::list<TSharedRef<FVoxelChunkSave>> SaveList;
	MainOctree->AddDirtyChunksToSaveList(SaveList);
	return FVoxelWorldSave(Depth, SaveList);
}

void VoxelData::LoadFromSaveAndGetModifiedPositions(FVoxelWorldSave Save, std::forward_list<FIntVector>& OutModifiedPositions, bool bReset)
{
	if (bReset)
	{
		MainOctree->GetDirtyChunksPositions(OutModifiedPositions);
		delete MainOctree;
		MainOctree = new ValueOctree(WorldGenerator, FIntVector::ZeroValue, Depth, Octree::GetTopIdFromDepth(Depth));
	}

	auto SaveList = Save.GetChunksList();
	MainOctree->LoadFromSaveAndGetModifiedPositions(SaveList, OutModifiedPositions);
	check(SaveList.empty());
}

void VoxelData::GetDiffArrays(std::forward_list<TArray<FVoxelValueDiff>>& OutValueDiffPacketsList, std::forward_list<TArray<FVoxelMaterialDiff>>& OutColorDiffPacketsList) const
{
	VoxelValueDiffArray ValueDiffArray;
	VoxelMaterialDiffArray ColorDiffArray;

	MainOctree->AddChunksToDiffArrays(ValueDiffArray, ColorDiffArray);

	ValueDiffArray.AddPackets(OutValueDiffPacketsList);
	ColorDiffArray.AddPackets(OutColorDiffPacketsList);
}

void VoxelData::LoadFromDiffArrayAndGetModifiedPositions(TArray<FVoxelValueDiff>& ValueDiffArray, TArray<FVoxelMaterialDiff>& ColorDiffArray, std::forward_list<FIntVector>& OutModifiedPositions)
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
