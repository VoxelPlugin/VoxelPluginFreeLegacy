// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelPrivatePCH.h"
#include "VoxelData.h"
#include "ValueOctree.h"

VoxelData::VoxelData(int Depth, UVoxelWorldGenerator* WorldGenerator) : Depth(Depth)
{
	MainOctree = MakeShareable(new ValueOctree(FIntVector::ZeroValue, Depth, WorldGenerator));
}

VoxelData::~VoxelData()
{

}

signed char VoxelData::GetValue(FIntVector Position)
{
	if (Position.X >= Size() / 2 || Position.Y >= Size() / 2 || Position.Z >= Size() / 2)
	{
		return GetValue(FIntVector(
			(Position.X >= Size() / 2) ? Size() / 2 - 1 : Position.X,
			(Position.Y >= Size() / 2) ? Size() / 2 - 1 : Position.Y,
			(Position.Z >= Size() / 2) ? Size() / 2 - 1 : Position.Z));
	}
	if (Position.X < -Size() / 2 || Position.Y < -Size() / 2 || Position.Z < -Size() / 2)
	{
		return GetValue(FIntVector(
			(Position.X < -Size() / 2) ? -Size() / 2 : Position.X,
			(Position.Y < -Size() / 2) ? -Size() / 2 : Position.Y,
			(Position.Z < -Size() / 2) ? -Size() / 2 : Position.Z));
	}

	if (IsInWorld(Position))
	{
		return MainOctree->GetValue(Position);
	}
	else
	{
		UE_LOG(VoxelLog, Fatal, TEXT("Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
		return 127;
	}
}

FColor VoxelData::GetColor(FIntVector Position)
{
	if (Position.X >= Size() / 2 || Position.Y >= Size() / 2 || Position.Z >= Size() / 2)
	{
		return GetColor(FIntVector(
			(Position.X >= Size() / 2) ? Size() / 2 - 1 : Position.X,
			(Position.Y >= Size() / 2) ? Size() / 2 - 1 : Position.Y,
			(Position.Z >= Size() / 2) ? Size() / 2 - 1 : Position.Z));
	}
	if (Position.X < -Size() / 2 || Position.Y < -Size() / 2 || Position.Z < -Size() / 2)
	{
		return GetColor(FIntVector(
			(Position.X < -Size() / 2) ? -Size() / 2 : Position.X,
			(Position.Y < -Size() / 2) ? -Size() / 2 : Position.Y,
			(Position.Z < -Size() / 2) ? -Size() / 2 : Position.Z));
	}

	if (IsInWorld(Position))
	{
		return MainOctree->GetColor(Position);
	}
	else
	{
		UE_LOG(VoxelLog, Fatal, TEXT("Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
		return FColor::Red;
	}
}

void VoxelData::SetValue(FIntVector Position, int Value)
{
	if (Position.X >= Size() / 2 || Position.Y >= Size() / 2 || Position.Z >= Size() / 2)
	{
		return;
	}
	if (Position.X < -Size() / 2 || Position.Y < -Size() / 2 || Position.Z < -Size() / 2)
	{
		return;
	}

	if (IsInWorld(Position))
	{
		MainOctree->SetValue(Position, FMath::Clamp(Value, -127, 127));
	}
	else
	{
		UE_LOG(VoxelLog, Fatal, TEXT("Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
	}
}

void VoxelData::SetColor(FIntVector Position, FColor Color)
{
	if (Position.X >= Size() / 2 || Position.Y >= Size() / 2 || Position.Z >= Size() / 2)
	{
		return;
	}
	if (Position.X < -Size() / 2 || Position.Y < -Size() / 2 || Position.Z < -Size() / 2)
	{
		return;
	}

	if (IsInWorld(Position))
	{
		MainOctree->SetColor(Position, Color);
	}
	else
	{
		UE_LOG(VoxelLog, Fatal, TEXT("Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
	}
}

bool VoxelData::IsInWorld(FIntVector Position)
{
	int  w = Size() / 2;
	return -w <= Position.X && Position.X < w && -w <= Position.Y && Position.Y < w && -w <= Position.Z && Position.Z < w;
}

int VoxelData::Size()
{
	return 16 << Depth;
}

TArray<FVoxelChunkSaveStruct> VoxelData::GetSaveArray()
{
	TArray<FVoxelChunkSaveStruct> SaveArray;
	MainOctree->AddChunksToArray(SaveArray);
	return SaveArray;
}

void VoxelData::LoadFromArray(TArray<FVoxelChunkSaveStruct> SaveArray)
{
	MainOctree->LoadFromArray(SaveArray);
}
