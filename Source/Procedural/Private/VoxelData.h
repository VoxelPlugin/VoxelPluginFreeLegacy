// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelChunkSaveStruct.h"

class ValueOctree;

class VoxelData
{
public:
	VoxelData(int Depth);
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
	
	virtual signed char GetDefaultValue(FIntVector Position) const;
	virtual FColor GetDefaultColor(FIntVector Position) const;

private:
	TSharedPtr<ValueOctree> MainOctree;
};
