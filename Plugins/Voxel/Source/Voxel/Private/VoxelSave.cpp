#pragma once

#include "VoxelPrivatePCH.h"
#include "CoreMinimal.h"
#include <list>
#include <forward_list>
#include "VoxelSave.h"



FVoxelChunkSave::FVoxelChunkSave() : Id(-1)
{

}

FVoxelChunkSave::FVoxelChunkSave(uint64 Id, FIntVector Position, TArray<float, TFixedAllocator<16 * 16 * 16>> Values, TArray<FColor, TFixedAllocator<16 * 16 * 16>>& Colors)
	: Id(Id), Values(Values), Colors(Colors)
{
}

FVoxelWorldSave::FVoxelWorldSave() : Depth(-1)
{

}

FVoxelWorldSave::FVoxelWorldSave(int Depth, std::list<FVoxelChunkSave> ChunksList) : Depth(Depth)
{
	Chunks.SetNum(ChunksList.size());

	for (int i = 0; i < Chunks.Num(); i++)
	{
		Chunks[i] = ChunksList.back();
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

FVoxelValueDiff::FVoxelValueDiff() : Id(-1), Index(-1), Value(0)
{

}

FVoxelValueDiff::FVoxelValueDiff(uint64 Id, int Index, float Value) : Id(Id), Index(Index), Value(Value)
{

}

FVoxelColorDiff::FVoxelColorDiff() : Id(-1), Index(-1), Color(FColor::Black)
{

}

FVoxelColorDiff::FVoxelColorDiff(uint64 Id, int Index, FColor Color) : Id(Id), Index(Index), Color(Color)
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

			DiffArray[Count - 1 - i] = *new FVoxelValueDiff(Ids.front(), Indexes.front(), Values.front());

			Ids.pop_front();
			Indexes.pop_front();
			Values.pop_front();
			Size--;
		}

		List.push_front(DiffArray);
	}
}

void VoxelColorDiffArray::Add(uint64 Id, int Index, FColor Color)
{
	Ids.push_front(Id);
	Indexes.push_front(Index);
	Colors.push_front(Color);
	Size++;
}

void VoxelColorDiffArray::AddPackets(std::forward_list<TArray<FVoxelColorDiff>>& List, const int MaxSize)
{
	const int MaxLength = MaxSize / 3;

	while (!Indexes.empty())
	{
		const int Count = FMath::Min(Size, MaxLength);

		TArray<FVoxelColorDiff> DiffArray;
		DiffArray.SetNumUninitialized(Count);

		for (int i = 0; i < Count; i++)
		{
			if (Ids.empty() || Indexes.empty() || Colors.empty())
			{
				break;
			}

			DiffArray[Count - 1 - i] = *new FVoxelColorDiff(Ids.front(), Indexes.front(), Colors.front());

			Ids.pop_front();
			Indexes.pop_front();
			Colors.pop_front();
			Size--;
		}

		List.push_front(DiffArray);
	}
}
