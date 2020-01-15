// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"

template<typename T>
struct TVoxelDiff
{
	FVoxelCellIndex Index;
	T Value;

	TVoxelDiff() = default;
	TVoxelDiff(FVoxelCellIndex Index, const T& Value) : Index(Index), Value(Value) {}
};

template<typename T>
FORCEINLINE FArchive& operator<<(FArchive &Ar, TVoxelDiff<T>& ValueDiff)
{
	Ar << ValueDiff.Index;
	Ar << ValueDiff.Value;

	return Ar;
}

template<>
FORCEINLINE FArchive& operator<<<FVoxelValue>(FArchive &Ar, TVoxelDiff<FVoxelValue>& ValueDiff)
{
	Ar << ValueDiff.Index;
	Ar << ValueDiff.Value.GetStorage();

	return Ar;
}

template<typename T>
struct TVoxelChunkDiff
{
	FIntVector Position;
	TArray<TVoxelDiff<T>> Diffs;

	TVoxelChunkDiff() = default;
	TVoxelChunkDiff(const FIntVector& Position) : Position(Position) {}
};

template<typename T>
FORCEINLINE FArchive& operator<<(FArchive &Ar, TVoxelChunkDiff<T>& ChunkDiff)
{
	Ar << ChunkDiff.Position;
	Ar << ChunkDiff.Diffs;

	return Ar;
}