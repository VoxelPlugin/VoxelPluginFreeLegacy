// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelContainers/VoxelStaticArray.h"
#include "VoxelMaterialIndices.generated.h"

USTRUCT() // UStruct to use it in TMap for GC
struct FVoxelMaterialIndices
{
	GENERATED_BODY()
	
	uint8 NumIndices = 0;
	uint8 CubicFace = 0;
	TVoxelStaticArray<uint8, 6> SortedIndices;

	FVoxelMaterialIndices() = default;

	FString ToString() const
	{
		FString Result;
		for (int32 Index = 0; Index < NumIndices; Index++)
		{
			if (!Result.IsEmpty()) Result += ", ";
			Result += FString::FromInt(SortedIndices[Index]);
		}
		return Result;
	}
	TArray<uint8> ToArray() const
	{
		return TArray<uint8>(SortedIndices.GetData(), NumIndices);
	}

	bool operator==(const FVoxelMaterialIndices& Other) const
	{
		if (NumIndices != Other.NumIndices) return false;
		if (CubicFace != Other.CubicFace) return false;
		for (int32 Index = 0; Index < NumIndices; Index++)
		{
			if (SortedIndices[Index] != Other.SortedIndices[Index]) return false;
		}
		return true;
	}
};

inline uint32 GetTypeHash(const FVoxelMaterialIndices& Indices)
{
	uint32 Hash = HashCombine(GetTypeHash(Indices.NumIndices), GetTypeHash(Indices.CubicFace));
	for (int32 Index = 0; Index < Indices.NumIndices; Index++)
	{
		Hash = HashCombine(Hash, GetTypeHash(Indices.SortedIndices[Index]));
	}
	return Hash;
}