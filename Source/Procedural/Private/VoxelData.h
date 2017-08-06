// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelChunkSaveStruct.h"

class ValueOctree;
class UVoxelWorldGenerator;

class VoxelData
{
public:
	VoxelData(int Depth, UVoxelWorldGenerator* WorldGenerator);
	virtual ~VoxelData();

	const int Depth;


	signed char GetValue(FIntVector Position);
	FColor GetColor(FIntVector Position);

	void SetValue(FIntVector Position, int Value);
	void SetColor(FIntVector Position, FColor Color);

	bool IsInWorld(FIntVector Position);

	int Size();

	TArray<FVoxelChunkSaveStruct> GetSaveArray();
	void LoadFromArray(TArray<FVoxelChunkSaveStruct> SaveArray);

private:
	TSharedPtr<ValueOctree> MainOctree;
};
