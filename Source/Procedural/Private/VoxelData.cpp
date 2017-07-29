// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelData.h"
#include "ValueOctree.h"
#include "EngineGlobals.h"
#include "Engine.h"

VoxelData::VoxelData(int depth) : Depth(depth)
{
	MainOctree = new ValueOctree(FIntVector::ZeroValue, depth);
	MainOctree->CreateTree();
}

VoxelData::~VoxelData()
{
	delete MainOctree;
}

signed char VoxelData::GetValue(FIntVector position)
{
	if (position.X == Size() / 2 || position.Y == Size() / 2 || position.Z == Size() / 2)
	{
		return GetValue(position - FIntVector(position.X == (Size() / 2), position.Y == (Size() / 2), position.Z == (Size() / 2)));
	}

	if (IsInWorld(position))
	{
		return MainOctree->GetValue(position);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Not in world: (%d, %d, %d)"), position.X, position.Y, position.Z));
		return 0;
	}
}

void VoxelData::SetValue(FIntVector position, int value)
{
	if (position.X == Size() / 2 || position.Y == Size() / 2 || position.Z == Size() / 2)
	{
		return;
	}

	if (IsInWorld(position))
	{
		MainOctree->SetValue(position, FMath::Clamp(value, -127, 127));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Not in world: (%d, %d, %d)"), position.X, position.Y, position.Z));
	}
}

bool VoxelData::IsInWorld(FIntVector position)
{
	int  w = Size() / 2;
	return -w <= position.X && position.X < w && -w <= position.Y && position.Y < w && -w <= position.Z && position.Z < w;
}

int VoxelData::Size()
{
	return 16 << Depth;
}