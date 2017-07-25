// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelData.h"
#include "ValueOctree.h"

VoxelData::VoxelData(int x, int y, int z, int depth)
{
	MainOctree = new ValueOctree(x, y, z, depth);
	MainOctree->CreateTree(FVector(0, 0, 0));
}

VoxelData::~VoxelData()
{
	delete MainOctree;
}

signed char VoxelData::GetValue(int x, int y, int z)
{
	return MainOctree->GetLeaf(x, y, z)->GetValue(x, y, z);
}

void VoxelData::SetValue(int x, int y, int z, signed char value)
{
	MainOctree->GetLeaf(x, y, z)->SetValue(x, y, z, value);
}
