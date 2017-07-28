// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class ValueOctree;

/**
 *
 */
class VoxelData
{
public:
	VoxelData(int depth);
	~VoxelData();

	const int Depth;


	signed char GetValue(FIntVector position);

	void SetValue(FIntVector position, int value);

	bool IsInWorld(FIntVector position);

	int Size();

private:
	ValueOctree* MainOctree;
};
