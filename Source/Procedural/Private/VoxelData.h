// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelChunkSaveStruct.h"

class ValueOctree;

DECLARE_LOG_CATEGORY_EXTERN(VoxelDataLog, Log, All);

class VoxelData
{
public:
	VoxelData(int depth);
	virtual ~VoxelData();

	const int Depth;


	signed char GetValue(FIntVector position);
	FColor GetColor(FIntVector position);

	void SetValue(FIntVector position, int value);
	void SetColor(FIntVector position, FColor color);

	bool IsInWorld(FIntVector position);

	int Size();

	TArray<FVoxelChunkSaveStruct> GetSaveArray();
	void LoadFromArray(TArray<FVoxelChunkSaveStruct> saveArray);
	
	virtual signed char GetDefaultValue(FIntVector position) const;
	virtual FColor GetDefaultColor(FIntVector position) const;

private:
	ValueOctree* MainOctree;
};
