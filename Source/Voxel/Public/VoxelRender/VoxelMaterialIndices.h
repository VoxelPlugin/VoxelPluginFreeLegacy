// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelContainers/VoxelStaticArray.h"
#include "VoxelMaterialIndices.generated.h"

USTRUCT() // UStruct to use it in TMap for GC
struct FVoxelMaterialIndices
{
	GENERATED_BODY()
	
	uint8 NumIndices = 0;
	TVoxelStaticArray<uint8, 6> SortedIndices;

	FVoxelMaterialIndices() = default;

	inline FString ToString() const
	{
		FString Result;
		for (int32 Index = 0; Index < NumIndices; Index++)
		{
			if (!Result.IsEmpty()) Result += ", ";
			Result += FString::FromInt(SortedIndices[Index]);
		}
		return Result;
	}

	inline bool operator==(const FVoxelMaterialIndices& Other) const
	{
		if (NumIndices != Other.NumIndices) return false;
		for (int32 Index = 0; Index < NumIndices; Index++)
		{
			if (SortedIndices[Index] != Other.SortedIndices[Index]) return false;
		}
		return true;
	}
};

inline uint32 GetTypeHash(const FVoxelMaterialIndices& Indices)
{
	uint32 Hash = GetTypeHash(Indices.NumIndices);
	for (int32 Index = 0; Index < Indices.NumIndices; Index++)
	{
		Hash = HashCombine(Hash, GetTypeHash(Indices.SortedIndices[Index]));
	}
	return Hash;
}