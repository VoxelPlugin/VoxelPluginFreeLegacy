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
	VoxelData(int x, int y, int z, int depth);
	~VoxelData();

	signed char GetValue(int x, int y, int z);

	void SetValue(int x, int y, int z, signed char value);

private:
	ValueOctree* MainOctree;
};
