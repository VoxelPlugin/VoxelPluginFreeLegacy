// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelData.h"
#include "ValueOctree.h"

VoxelData::VoxelData(int Depth, AVoxelWorldGenerator* WorldGenerator, bool bMultiplayer) : Depth(Depth)
{
	MainOctree = MakeShareable(new ValueOctree(bMultiplayer, WorldGenerator, FIntVector::ZeroValue, Depth));
}

int VoxelData::Width() const
{
	return 16 << Depth;
}

TSharedPtr<ValueOctree> VoxelData::GetValueOctree() const
{
	return TSharedPtr<ValueOctree>(MainOctree);
}

float VoxelData::GetValue(FIntVector Position) const
{
	check(IsInWorld(Position));
	return GetValue(Position.X, Position.Y, Position.Z);
}

float VoxelData::GetValue(int X, int Y, int Z) const
{
	return MainOctree->GetValue(FMath::Clamp(X, -Width() / 2, Width() / 2 - 1), FMath::Clamp(Y, -Width() / 2, Width() / 2 - 1), FMath::Clamp(Z, -Width() / 2, Width() / 2 - 1));
}

FColor VoxelData::GetColor(FIntVector Position) const
{
	check(IsInWorld(Position));
	return GetColor(Position.X, Position.Y, Position.Z);
}

FColor VoxelData::GetColor(int X, int Y, int Z) const
{
	return MainOctree->GetColor(FMath::Clamp(X, -Width() / 2, Width() / 2 - 1), FMath::Clamp(Y, -Width() / 2, Width() / 2 - 1), FMath::Clamp(Z, -Width() / 2, Width() / 2 - 1));
}

void VoxelData::SetValue(FIntVector Position, float Value) const
{
	if (Position.X >= Width() / 2 || Position.Y >= Width() / 2 || Position.Z >= Width() / 2)
	{
		return;
	}
	if (Position.X < -Width() / 2 || Position.Y < -Width() / 2 || Position.Z < -Width() / 2)
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
	if (Position.X >= Width() / 2 || Position.Y >= Width() / 2 || Position.Z >= Width() / 2)
	{
		return;
	}
	if (Position.X < -Width() / 2 || Position.Y < -Width() / 2 || Position.Z < -Width() / 2)
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
	int  w = Width() / 2;
	return -w <= Position.X && Position.X < w && -w <= Position.Y && Position.Y < w && -w <= Position.Z && Position.Z < w;
}

FVoxelWorldSave VoxelData::GetSave() const
{
	std::list<FVoxelChunkSave> SaveArray;
	MainOctree->AddChunksToArray(SaveArray);
	return FVoxelWorldSave(Depth, SaveArray);
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
