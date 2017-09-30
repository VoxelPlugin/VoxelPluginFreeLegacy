#pragma once

#include "VoxelPrivatePCH.h"
#include "VoxelSave.h"



FVoxelChunkSave::FVoxelChunkSave() 
	: Id(-1)
{

}

FVoxelChunkSave::FVoxelChunkSave(uint64 Id, FIntVector Position, TArray<float, TFixedAllocator<16 * 16 * 16>> Values, TArray<FVoxelMaterial, TFixedAllocator<16 * 16 * 16>>& Materials)
	: Id(Id)
	, Values(Values)
	, Materials(Materials)
{
}

FVoxelWorldSave::FVoxelWorldSave() 
	: Depth(-1)
{

}

FVoxelWorldSave::FVoxelWorldSave(int Depth, std::list<TSharedRef<FVoxelChunkSave>> ChunksList) 
	: Depth(Depth)
{
	Chunks.SetNum(ChunksList.size());

	for (int i = 0; i < Chunks.Num(); i++)
	{
		Chunks[i] = *ChunksList.back();
		ChunksList.pop_back();
	}
}

std::list<FVoxelChunkSave> FVoxelWorldSave::GetChunksList()
{
	std::list<FVoxelChunkSave> ChunksList;
	for (int i = 0; i < Chunks.Num(); i++)
	{
		ChunksList.push_front(Chunks[i]);
	}
	return ChunksList;
}

FVoxelValueDiff::FVoxelValueDiff() 
	: Id(-1)
	, Index(-1)
	, Value(0)
{

}

FVoxelValueDiff::FVoxelValueDiff(uint64 Id, int Index, float Value) 
	: Id(Id)
	, Index(Index)
	, Value(Value)
{

}

FVoxelMaterialDiff::FVoxelMaterialDiff() 
	: Id(-1)
	, Index(-1)
	, Material()
{

}

FVoxelMaterialDiff::FVoxelMaterialDiff(uint64 Id, int Index, FVoxelMaterial Material) 
	: Id(Id)
	, Index(Index)
	, Material(Material)
{

}

void VoxelValueDiffArray::Add(uint64 Id, int Index, float Value)
{
	Ids.push_front(Id);
	Indexes.push_front(Index);
	Values.push_front(Value);
	Size++;
}

void VoxelValueDiffArray::AddPackets(std::forward_list<TArray<FVoxelValueDiff>>& List, const int MaxSize)
{
	const int MaxLength = MaxSize / 3;

	while (!Indexes.empty())
	{
		const int Count = FMath::Min(Size, MaxLength);

		TArray<FVoxelValueDiff> DiffArray;
		DiffArray.SetNumUninitialized(Count);

		for (int i = 0; i < Count; i++)
		{
			if (Ids.empty() || Indexes.empty() || Values.empty())
			{
				break;
			}

			// TODO: Leak?
			DiffArray[Count - 1 - i] = *new FVoxelValueDiff(Ids.front(), Indexes.front(), Values.front());

			Ids.pop_front();
			Indexes.pop_front();
			Values.pop_front();
			Size--;
		}

		List.push_front(DiffArray);
	}
}

void VoxelMaterialDiffArray::Add(uint64 Id, int Index, FVoxelMaterial Material)
{
	Ids.push_front(Id);
	Indexes.push_front(Index);
	Materials.push_front(Material);
	Size++;
}

void VoxelMaterialDiffArray::AddPackets(std::forward_list<TArray<FVoxelMaterialDiff>>& List, const int MaxSize)
{
	const int MaxLength = MaxSize / 3;

	while (!Indexes.empty())
	{
		const int Count = FMath::Min(Size, MaxLength);

		TArray<FVoxelMaterialDiff> DiffArray;
		DiffArray.SetNumUninitialized(Count);

		for (int i = 0; i < Count; i++)
		{
			if (Ids.empty() || Indexes.empty() || Materials.empty())
			{
				break;
			}

			// TODO: Leak?
			DiffArray[Count - 1 - i] = *new FVoxelMaterialDiff(Ids.front(), Indexes.front(), Materials.front());

			Ids.pop_front();
			Indexes.pop_front();
			Materials.pop_front();
			Size--;
		}

		List.push_front(DiffArray);
	}
}
