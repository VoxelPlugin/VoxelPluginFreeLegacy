// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class ValueOctree;

DECLARE_LOG_CATEGORY_EXTERN(VoxelDataLog, Log, All);

UCLASS()
class VoxelData
{
	GENERATED_BODY()
public:
	VoxelData(int depth);
	~VoxelData();

	const int Depth;


	signed char GetValue(FIntVector position);
	FColor GetColor(FIntVector position);

	void SetValue(FIntVector position, int value);
	void SetColor(FIntVector position, FColor color);

	bool IsInWorld(FIntVector position);

	int Size();
	
	virtual signed char GetDefaultValue(FIntVector position);
	virtual FColor GetDefaultColor(FIntVector position);

private:
	ValueOctree* MainOctree;
};
