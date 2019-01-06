// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelId.h"
#include "VoxelGlobals.h"

template<typename T>
struct TVoxelDiff
{
	FVoxelCellIndex Index;
	T Value;

	TVoxelDiff() = default;
	TVoxelDiff(FVoxelCellIndex Index, const T& Value) : Index(Index), Value(Value) {}
};

template<typename T>
inline FArchive& operator<<(FArchive &Ar, TVoxelDiff<T>& ValueDiff)
{
	Ar << ValueDiff.Index;
	Ar << ValueDiff.Value;

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
inline FArchive& operator<<(FArchive &Ar, TVoxelChunkDiff<T>& ChunkDiff)
{
	Ar << ChunkDiff.Position;
	Ar << ChunkDiff.Diffs;

	return Ar;
}