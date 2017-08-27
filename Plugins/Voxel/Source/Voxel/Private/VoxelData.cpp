// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelData.h"
#include "ValueOctree.h"

VoxelData::VoxelData(int Depth, TScriptInterface<IVoxelWorldGenerator>& WorldGenerator, bool bMultiplayer) : Depth(Depth)
{
	MainOctree = MakeShareable(new ValueOctree(bMultiplayer, WorldGenerator, FIntVector::ZeroValue, Depth));
}

TSharedPtr<ValueOctree> VoxelData::GetValueOctree() const
{
	return TSharedPtr<ValueOctree>(MainOctree);
}

float VoxelData::GetValue(FIntVector Position) const
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
		return 0;
	}
}

FColor VoxelData::GetColor(FIntVector Position) const
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

void VoxelData::SetValue(FIntVector Position, float Value) const
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
		MainOctree->SetValue(Position, Value);
	}
	else
	{
		UE_LOG(VoxelLog, Fatal, TEXT("Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
	}
}

void VoxelData::SetColor(FIntVector Position, FColor Color) const
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

bool VoxelData::IsInWorld(FIntVector Position) const
{
	int  w = Size() / 2;
	return -w <= Position.X && Position.X < w && -w <= Position.Y && Position.Y < w && -w <= Position.Z && Position.Z < w;
}

int VoxelData::Size() const
{
	return 16 << Depth;
}

std::list<FVoxelChunkSave> VoxelData::GetSaveArray() const
{
	std::list<FVoxelChunkSave> SaveArray;
	MainOctree->AddChunksToArray(SaveArray);
	return SaveArray;
}

void VoxelData::LoadAndQueueUpdateFromSave(std::list<FVoxelChunkSave>& SaveArray, AVoxelWorld* World, bool bReset)
{
	check(World);
	if (bReset)
	{
		MainOctree->QueueUpdateOfDirtyChunks(World);
		MainOctree = MakeShareable(new ValueOctree(MainOctree->bMultiplayer, MainOctree->WorldGenerator, FIntVector::ZeroValue, Depth));
		World->ApplyQueuedUpdates(false);
	}
	MainOctree->LoadAndQueueUpdateFromSave(SaveArray, World);
	check(SaveArray.empty());
}

void VoxelData::GetDiffArrays(std::forward_list<TArray<FVoxelValueDiff>>& OutValueDiffPacketsList, std::forward_list<TArray<FVoxelColorDiff>>& OutColorDiffPacketsList) const
{
	VoxelValueDiffArray ValueDiffArray;
	VoxelColorDiffArray ColorDiffArray;

	MainOctree->AddChunksToDiffArrays(ValueDiffArray, ColorDiffArray);

	ValueDiffArray.AddPackets(OutValueDiffPacketsList);
	ColorDiffArray.AddPackets(OutColorDiffPacketsList);
}

void VoxelData::LoadAndQueueUpdateFromDiffArray(const TArray<FVoxelValueDiff>& ValueDiffArray, const TArray<FVoxelColorDiff>& ColorDiffArray, AVoxelWorld* World) const
{
	check(World);
	std::forward_list<FVoxelValueDiff> ValueDiffList;
	std::forward_list<FVoxelColorDiff> ColorDiffList;

	for (int i = ValueDiffArray.Num() - 1; i >= 0; i--)
	{
		ValueDiffList.push_front(ValueDiffArray[i]);
	}
	for (int i = ColorDiffArray.Num() - 1; i >= 0; i--)
	{
		ColorDiffList.push_front(ColorDiffArray[i]);
	}

	MainOctree->LoadAndQueueUpdateFromDiffArrays(ValueDiffList, ColorDiffList, World);
}
