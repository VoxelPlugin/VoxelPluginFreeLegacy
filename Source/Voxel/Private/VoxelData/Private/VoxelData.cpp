// Copyright 2018 Phyronnaz

#include "VoxelData.h"
#include "VoxelPrivate.h"
#include "ValueOctree.h"
#include "VoxelSave.h"
#include "VoxelDiff.h"
#include "VoxelWorldGenerator.h"
#include "Algo/Reverse.h"
#include "ScopeLock.h"

FVoxelData::FVoxelData(int LOD, TSharedRef<FVoxelWorldGeneratorInstance> WorldGenerator, bool bMultiplayer)
	: LOD(LOD)
	, WorldGenerator(WorldGenerator)
	, bMultiplayer(bMultiplayer)
	, MainOctree(new FValueOctree(WorldGenerator, LOD, bMultiplayer))
{
}

FVoxelData::~FVoxelData()
{
	delete MainOctree;
}

int32 FVoxelData::Size() const
{
	return DATA_CHUNK_SIZE << LOD;
}

FIntBox FVoxelData::GetBounds() const
{
	return MainOctree->GetBounds();
}

TArray<uint64> FVoxelData::BeginSet(const FIntBox& Box)
{
	TArray<uint64> LockedOctrees;

	MainOctree->LockTransactions();
	MainOctree->BeginSet(Box, LockedOctrees);

	Algo::Reverse(LockedOctrees);
	return LockedOctrees;
}

void FVoxelData::EndSet(TArray<uint64>& LockedOctrees)
{
	MainOctree->EndSet(LockedOctrees);
	check(LockedOctrees.Num() == 0);
}

TArray<uint64> FVoxelData::BeginGet(const FIntBox& Box)
{
	TArray<uint64> LockedOctrees;

	MainOctree->LockTransactions();
	MainOctree->BeginGet(Box, LockedOctrees);

	Algo::Reverse(LockedOctrees);
	return LockedOctrees;
}

void FVoxelData::EndGet(TArray<uint64>& LockedOctrees)
{
	MainOctree->EndGet(LockedOctrees);
	check(LockedOctrees.Num() == 0);
}

bool FVoxelData::IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const
{
	return MainOctree->IsEmpty(Start, Step, Size);
}


void FVoxelData::GetValuesAndMaterials(float Values[], FVoxelMaterial Materials[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& InSize, const FIntVector& ArraySize) const
{
	if (InSize.X <= 0 || InSize.Y <= 0 || InSize.Z <= 0)
	{
		return;
	}
	if (UNLIKELY(!IsInWorld(Start.X, Start.Y, Start.Z) || !IsInWorld(Start.X + (InSize.X - 1) * Step, Start.Y + (InSize.Y - 1) * Step, Start.Z + (InSize.Z - 1) * Step)))
	{
		for (int X = 0; X < InSize.X; X++)
		{
			for (int Y = 0; Y < InSize.Y; Y++)
			{
				for (int Z = 0; Z < InSize.Z; Z++)
				{
					const int Index = (StartIndex.X + X) + ArraySize.X * (StartIndex.Y + Y) + ArraySize.X * ArraySize.Y * (StartIndex.Z + Z);
					int RX = Start.X + X * Step;
					int RY = Start.Y + Y * Step;
					int RZ = Start.Z + Z * Step;
					if (Values)
					{
						if (IsInWorld(RX, RY, RZ))
						{
							Values[Index] = GetValue(RX, RY, RZ);
						}
						else
						{
							Values[Index] = WorldGenerator->GetValue(RX, RY, RZ);
						}
					}
					if (Materials)
					{
						if (IsInWorld(RX, RY, RZ))
						{
							Materials[Index] = GetMaterial(RX, RY, RZ);
						}
						else
						{
							Materials[Index] = WorldGenerator->GetMaterial(RX, RY, RZ);
						}
					}
				}
			}
		}
	}
	else
	{
		MainOctree->GetValuesAndMaterials(Values, Materials, Start, StartIndex, Step, InSize, ArraySize);
	}
}

float FVoxelData::GetValue(int X, int Y, int Z) const
{
	return GetValue(FIntVector(X, Y, Z));
}
float FVoxelData::GetValue(const FIntVector& P) const
{
	float Value;
	GetValuesAndMaterials(&Value, nullptr, P, FIntVector::ZeroValue, 1, FIntVector(1, 1, 1), FIntVector(1, 1, 1));
	return Value;
}

FVoxelMaterial FVoxelData::GetMaterial(int X, int Y, int Z) const
{
	return GetMaterial(FIntVector(X, Y, Z));
}
FVoxelMaterial FVoxelData::GetMaterial(const FIntVector& P) const
{
	FVoxelMaterial Material;
	GetValuesAndMaterials(nullptr, &Material, P, FIntVector::ZeroValue, 1, FIntVector(1, 1, 1), FIntVector(1, 1, 1));
	return Material;
}

void FVoxelData::GetValueAndMaterial(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial) const
{
	GetValueAndMaterial(FIntVector(X, Y, Z), OutValue, OutMaterial);
}
void FVoxelData::GetValueAndMaterial(const FIntVector& P, float& OutValue, FVoxelMaterial& OutMaterial) const
{
	GetValuesAndMaterials(&OutValue, &OutMaterial, P, FIntVector::ZeroValue, 1, FIntVector(1, 1, 1), FIntVector(1, 1, 1));
}

FVector FVoxelData::GetGradient(int X, int Y, int Z)
{	
	FVector Gradient;
	Gradient.X = GetValue(X + 1, Y, Z) - GetValue(X - 1, Y, Z);
	Gradient.Y = GetValue(X, Y + 1, Z) - GetValue(X, Y - 1, Z);
	Gradient.Z = GetValue(X, Y, Z + 1) - GetValue(X, Y, Z - 1);
	return Gradient.GetSafeNormal();
}

FVector FVoxelData::GetGradient(const FIntVector& P)
{
	return GetGradient(P.X, P.Y, P.Z);
}

void FVoxelData::SetValue(int X, int Y, int Z, float Value)
{
	check(IsInWorld(X, Y, Z));
	MainOctree->GetLeaf(X, Y, Z)->SetValueAndMaterial(X, Y, Z, Value, FVoxelMaterial(), true, false);
}
void FVoxelData::SetValue(const FIntVector& P, float Value)
{
	SetValue(P.X, P.Y, P.Z, Value);
}

void FVoxelData::SetValue(int X, int Y, int Z, float Value, FValueOctree*& LastOctree)
{
	check(IsInWorld(X, Y, Z));
	if (UNLIKELY(!LastOctree || !LastOctree->IsLeaf() || !LastOctree->IsInOctree(X, Y, Z)))
	{
		LastOctree = MainOctree->GetLeaf(X, Y, Z);
	}
	LastOctree->SetValueAndMaterial(X, Y, Z, Value, FVoxelMaterial(), true, false);
}
void FVoxelData::SetValue(const FIntVector& P, float Value, FValueOctree*& LastOctree)
{
	SetValue(P.X, P.Y, P.Z, Value, LastOctree);
}

void FVoxelData::SetMaterial(int X, int Y, int Z, FVoxelMaterial Material)
{
	check(IsInWorld(X, Y, Z));
	MainOctree->GetLeaf(X, Y, Z)->SetValueAndMaterial(X, Y, Z, 0, Material, false, true);
}
void FVoxelData::SetMaterial(const FIntVector& P, FVoxelMaterial Material)
{
	SetMaterial(P.X, P.Y, P.Z, Material);
}

void FVoxelData::SetMaterial(int X, int Y, int Z, FVoxelMaterial Material, FValueOctree*& LastOctree)
{
	check(IsInWorld(X, Y, Z));
	if (UNLIKELY(!LastOctree || !LastOctree->IsLeaf() || !LastOctree->IsInOctree(X, Y, Z)))
	{
		LastOctree = MainOctree->GetLeaf(X, Y, Z);
	}
	LastOctree->SetValueAndMaterial(X, Y, Z, 0, Material, false, true);
}
void FVoxelData::SetMaterial(const FIntVector& P, FVoxelMaterial Material, FValueOctree*& LastOctree)
{
	SetMaterial(P.X, P.Y, P.Z, Material, LastOctree);
}

void FVoxelData::SetValueAndMaterial(int X, int Y, int Z, float Value, FVoxelMaterial Material, FValueOctree*& LastOctree)
{
	check(IsInWorld(X, Y, Z));
	if (UNLIKELY(!LastOctree || !LastOctree->IsLeaf() || !LastOctree->IsInOctree(X, Y, Z)))
	{
		LastOctree = MainOctree->GetLeaf(X, Y, Z);
	}
	LastOctree->SetValueAndMaterial(X, Y, Z, Value, Material, true, true);
}
void FVoxelData::SetValueAndMaterial(const FIntVector& P, float Value, FVoxelMaterial Material, FValueOctree*& LastOctree)
{
	SetValueAndMaterial(P.X, P.Y, P.Z, Value, Material, LastOctree);
}


bool FVoxelData::IsInWorld(int X, int Y, int Z) const
{
	int32 S = Size() / 2;
	return -S < X && X < S
		&& -S < Y && Y < S
		&& -S < Z && Z < S;
}
bool FVoxelData::IsInWorld(const FIntVector& P) const
{
	return IsInWorld(P.X, P.Y, P.Z);
}

FORCEINLINE void FVoxelData::ClampToWorld(int& X, int& Y, int& Z) const
{
	int32 S = Size() / 2;
	X = FMath::Clamp(X, -S, S - 1);
	Y = FMath::Clamp(Y, -S, S - 1);
	Z = FMath::Clamp(Z, -S, S - 1);
}

void FVoxelData::GetSave(FVoxelWorldSave& OutSave)
{
	auto Octrees = BeginGet(FIntBox::Infinite());

	TArray<FVoxelChunkSave> SaveQueue;
	MainOctree->AddDirtyChunksToSaveQueue(SaveQueue);
	OutSave.Init(LOD, SaveQueue);

	EndGet(Octrees);
}

void FVoxelData::LoadFromSaveAndGetModifiedPositions(const FVoxelWorldSave& Save, TArray<FIntVector>& OutModifiedPositions, bool bReset)
{
	auto Octrees = BeginSet(FIntBox::Infinite());

	if (bReset)
	{
		TArray<FValueOctree*> Leaves;
		MainOctree->GetLeavesOverlappingBox(FIntBox::Infinite(), Leaves);
		for (auto& Leaf : Leaves)
		{
			if (Leaf->LOD == 0 && Leaf->IsDirty())
			{
				Leaf->SetAsNotDirty();
			}
		}
	}

	TArray<FVoxelChunkSave> SaveQueue;
	Save.GetChunksQueue(SaveQueue);
	Algo::Reverse(SaveQueue);

	MainOctree->LoadFromSaveQueueAndGetModifiedPositions(SaveQueue, OutModifiedPositions);
	check(SaveQueue.Num() == 0);

	EndSet(Octrees);
}


void FVoxelData::SetWorldGenerator(TSharedRef<FVoxelWorldGeneratorInstance> NewGenerator)
{
	auto Octrees = BeginSet(FIntBox::Infinite());

	MainOctree->SetWorldGenerator(NewGenerator);
	WorldGenerator = NewGenerator;
	
	EndSet(Octrees);
}

void FVoxelData::DiscardValuesByPredicateF(const std::function<int(const FIntBox&)>& P)
{
	auto Octrees = BeginSet(FIntBox::Infinite());

	MainOctree->DiscardValuesByPredicate(P);
	
	EndSet(Octrees);
}
