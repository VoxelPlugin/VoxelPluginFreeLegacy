// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelData.h"
#include "ValueOctree.h"
#include "EngineGlobals.h"
#include "Engine.h"

DEFINE_LOG_CATEGORY(VoxelDataLog);

VoxelData::VoxelData(int depth) : Depth(depth)
{
	MainOctree = MakeShareable(new ValueOctree(FIntVector::ZeroValue, depth, this));
	MainOctree->CreateTree();
}

VoxelData::~VoxelData()
{

}

signed char VoxelData::GetValue(FIntVector position)
{
	if (position.X >= Size() / 2 || position.Y >= Size() / 2 || position.Z >= Size() / 2)
	{
		return GetValue(FIntVector(
			(position.X >= Size() / 2) ? Size() / 2 - 1 : position.X,
			(position.Y >= Size() / 2) ? Size() / 2 - 1 : position.Y,
			(position.Z >= Size() / 2) ? Size() / 2 - 1 : position.Z));
	}
	if (position.X < -Size() / 2 || position.Y < -Size() / 2 || position.Z < -Size() / 2)
	{
		return GetValue(FIntVector(
			(position.X < -Size() / 2) ? -Size() / 2 : position.X,
			(position.Y < -Size() / 2) ? -Size() / 2 : position.Y,
			(position.Z < -Size() / 2) ? -Size() / 2 : position.Z));
	}

	if (IsInWorld(position))
	{
		return MainOctree->GetValue(position);
	}
	else
	{
		UE_LOG(VoxelDataLog, Fatal, TEXT("Not in world: (%d, %d, %d)"), position.X, position.Y, position.Z);
		return 127;
	}
}

FColor VoxelData::GetColor(FIntVector position)
{
	if (position.X >= Size() / 2 || position.Y >= Size() / 2 || position.Z >= Size() / 2)
	{
		return GetColor(FIntVector(
			(position.X >= Size() / 2) ? Size() / 2 - 1 : position.X,
			(position.Y >= Size() / 2) ? Size() / 2 - 1 : position.Y,
			(position.Z >= Size() / 2) ? Size() / 2 - 1 : position.Z));
	}
	if (position.X < -Size() / 2 || position.Y < -Size() / 2 || position.Z < -Size() / 2)
	{
		return GetColor(FIntVector(
			(position.X < -Size() / 2) ? -Size() / 2 : position.X,
			(position.Y < -Size() / 2) ? -Size() / 2 : position.Y,
			(position.Z < -Size() / 2) ? -Size() / 2 : position.Z));
	}

	if (IsInWorld(position))
	{
		return MainOctree->GetColor(position);
	}
	else
	{
		UE_LOG(VoxelDataLog, Fatal, TEXT("Not in world: (%d, %d, %d)"), position.X, position.Y, position.Z);
		return FColor::Red;
	}
}

void VoxelData::SetValue(FIntVector position, int value)
{
	if (position.X >= Size() / 2 || position.Y >= Size() / 2 || position.Z >= Size() / 2)
	{
		return;
	}
	if (position.X < -Size() / 2 || position.Y < -Size() / 2 || position.Z < -Size() / 2)
	{
		return;
	}

	if (IsInWorld(position))
	{
		MainOctree->SetValue(position, FMath::Clamp(value, -127, 127));
	}
	else
	{
		UE_LOG(VoxelDataLog, Error, TEXT("Not in world: (%d, %d, %d)"), position.X, position.Y, position.Z);
	}
}

void VoxelData::SetColor(FIntVector position, FColor color)
{
	if (position.X >= Size() / 2 || position.Y >= Size() / 2 || position.Z >= Size() / 2)
	{
		return;
	}
	if (position.X < -Size() / 2 || position.Y < -Size() / 2 || position.Z < -Size() / 2)
	{
		return;
	}

	if (IsInWorld(position))
	{
		MainOctree->SetColor(position, color);
	}
	else
	{
		UE_LOG(VoxelDataLog, Error, TEXT("Not in world: (%d, %d, %d)"), position.X, position.Y, position.Z);
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

signed char VoxelData::GetDefaultValue(FIntVector position) const
{
	return (position.Z == 8) ? 0 : ((position.Z > 8) ? 100 : -100);
}

FColor VoxelData::GetDefaultColor(FIntVector position) const
{
	return (position.Z == 8) ? FColor::White : ((position.Z > 8) ? FColor::Red : FColor::Green);
}

TArray<FVoxelChunkSaveStruct> VoxelData::GetSaveArray()
{
	TArray<FVoxelChunkSaveStruct> SaveArray;
	MainOctree->AddChunksToArray(SaveArray);
	return SaveArray;
}

void VoxelData::LoadFromArray(TArray<FVoxelChunkSaveStruct> saveArray)
{
	MainOctree->LoadFromArray(saveArray);
}
