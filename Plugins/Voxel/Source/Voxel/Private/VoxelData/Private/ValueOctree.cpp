#include "VoxelPrivatePCH.h"
#include "ValueOctree.h"
#include "VoxelData.h"
#include "VoxelWorldGenerator.h"
#include "GenericPlatformProcess.h"

ValueOctree::ValueOctree(AVoxelWorldGenerator* WorldGenerator, FIntVector Position, uint8 Depth, uint64 Id)
	: Octree(Position, Depth, Id)
	, WorldGenerator(WorldGenerator)
	, bIsDirty(false)
	, bIsNetworkDirty(false)
{
	GetLock = FGenericPlatformProcess::GetSynchEventFromPool(true);
}

bool ValueOctree::IsDirty() const
{
	return bIsDirty;
}

void ValueOctree::GetValueAndColor(int X, int Y, int Z, float& OutValue, FColor& OutColor)
{
	check(IsLeaf());
	check(IsInOctree(X, Y, Z));

	if (UNLIKELY(IsDirty()))
	{
		check(Depth == 0);

		GetLock->Wait();

		int LocalX, LocalY, LocalZ;
		GlobalToLocal(X, Y, Z, LocalX, LocalY, LocalZ);

		int Index = IndexFromCoordinates(LocalX, LocalY, LocalZ);
		OutValue = Values[Index];
		OutColor = Colors[Index];
	}
	else
	{
		OutValue = WorldGenerator->GetDefaultValue(X, Y, Z);
		OutColor = WorldGenerator->GetDefaultColor(X, Y, Z);
	}
}

void ValueOctree::SetValue(int X, int Y, int Z, float Value)
{
	check(IsLeaf());
	check(IsInOctree(X, Y, Z));

	bIsNetworkDirty = true;

	if (Depth != 0)
	{
		CreateChilds();
		bIsDirty = true; // IsDirty only when having childs (for multithreading)
		GetChild(X, Y, Z)->SetValue(X, Y, Z, Value);
	}
	else
	{
		FScopeLock Lock(&SetLock);
		GetLock->Reset();

		if (!IsDirty())
		{
			SetAsDirty();
		}

		int LocalX, LocalY, LocalZ;
		GlobalToLocal(X, Y, Z, LocalX, LocalY, LocalZ);

		int Index = IndexFromCoordinates(LocalX, LocalY, LocalZ);
		Values[Index] = Value;

		if (bMultiplayer)
		{
			DirtyValues.Add(Index);
		}

		GetLock->Trigger();
	}
}

void ValueOctree::SetColor(int X, int Y, int Z, FColor Color)
{
	check(IsLeaf());
	check(IsInOctree(X, Y, Z));

	bIsNetworkDirty = true;

	if (Depth != 0)
	{
		CreateChilds();
		bIsDirty = true; // IsDirty only when having childs (for multithreading)
		GetChild(X, Y, Z)->SetColor(X, Y, Z, Color);
	}
	else
	{
		FScopeLock Lock(&SetLock);
		GetLock->Reset();

		if (!IsDirty())
		{
			SetAsDirty();
		}

		int LocalX, LocalY, LocalZ;
		GlobalToLocal(X, Y, Z, LocalX, LocalY, LocalZ);

		int Index = IndexFromCoordinates(LocalX, LocalY, LocalZ);
		Colors[Index] = Color;

		if (bMultiplayer)
		{
			DirtyValues.Add(Index);
		}

		GetLock->Trigger();
	}
}

void ValueOctree::AddDirtyChunksToSaveList(std::list<TSharedRef<FVoxelChunkSave>>& SaveList)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(!(IsDirty() && IsLeaf() && Depth != 0));

	if (IsDirty())
	{
		if (IsLeaf())
		{
			auto SaveStruct = TSharedRef<FVoxelChunkSave>(new FVoxelChunkSave(Id, Position, Values, Colors));
			SaveList.push_back(SaveStruct);
		}
		else
		{
			for (auto Child : Childs)
			{
				Child->AddDirtyChunksToSaveList(SaveList);
			}
		}
	}
}

void ValueOctree::LoadFromSaveAndGetModifiedPositions(std::list<FVoxelChunkSave>& Save, std::forward_list<FIntVector>& OutModifiedPositions)
{
	if (Save.empty())
	{
		return;
	}

	if (Depth == 0)
	{
		if (Save.front().Id == Id)
		{
			bIsDirty = true;
			Values = Save.front().Values;
			Colors = Save.front().Colors;
			Save.pop_front();

			check(Values.Num() == 4096);
			check(Colors.Num() == 4096);

			// Update neighbors
			const int S = Size();
			OutModifiedPositions.push_front(Position - FIntVector(0, 0, 0));
			OutModifiedPositions.push_front(Position - FIntVector(S, 0, 0));
			OutModifiedPositions.push_front(Position - FIntVector(0, S, 0));
			OutModifiedPositions.push_front(Position - FIntVector(S, S, 0));
			OutModifiedPositions.push_front(Position - FIntVector(0, 0, S));
			OutModifiedPositions.push_front(Position - FIntVector(S, 0, S));
			OutModifiedPositions.push_front(Position - FIntVector(0, S, S));
			OutModifiedPositions.push_front(Position - FIntVector(S, S, S));
		}
	}
	else
	{
		uint64 Pow = IntPow9(Depth);
		if (Id / Pow == Save.front().Id / Pow)
		{
			if (IsLeaf())
			{
				CreateChilds();
				bIsDirty = true;
			}
			for (auto Child : Childs)
			{
				Child->LoadFromSaveAndGetModifiedPositions(Save, OutModifiedPositions);
			}
		}
	}
}

void ValueOctree::AddChunksToDiffArrays(VoxelValueDiffArray& ValuesDiffs, VoxelColorDiffArray& ColorsDiffs)
{
	if (bIsNetworkDirty)
	{
		bIsNetworkDirty = false;

		if (IsLeaf())
		{
			for (int Index : DirtyValues)
			{
				ValuesDiffs.Add(Id, Index, Values[Index]);
			}
			for (int Index : DirtyColors)
			{
				ColorsDiffs.Add(Id, Index, Colors[Index]);
			}
			DirtyValues.Empty(4096);
			DirtyColors.Empty(4096);
		}
		else
		{
			for (auto Child : Childs)
			{
				Child->AddChunksToDiffArrays(ValuesDiffs, ColorsDiffs);
			}
		}
	}
}

void ValueOctree::LoadFromDiffListAndGetModifiedPositions(std::forward_list<FVoxelValueDiff>& ValuesDiffs, std::forward_list<FVoxelColorDiff>& ColorsDiffs, std::forward_list<FIntVector>& OutModifiedPositions)
{
	if (ValuesDiffs.empty() && ColorsDiffs.empty())
	{
		return;
	}

	if (Depth == 0)
	{
		// Values
		while (!ValuesDiffs.empty() && ValuesDiffs.front().Id == Id)
		{
			if (!IsDirty())
			{
				SetAsDirty();
			}

			Values[ValuesDiffs.front().Index] = ValuesDiffs.front().Value;

			int X, Y, Z;
			CoordinatesFromIndex(ValuesDiffs.front().Index, X, Y, Z);
			OutModifiedPositions.push_front(FIntVector(X, Y, Z) + Position);

			ValuesDiffs.pop_front();
		}
		// Colors
		while (!ColorsDiffs.empty() && ColorsDiffs.front().Id == Id)
		{
			if (!IsDirty())
			{
				SetAsDirty();
			}

			Colors[ColorsDiffs.front().Index] = ColorsDiffs.front().Color;

			int X, Y, Z;
			CoordinatesFromIndex(ColorsDiffs.front().Index, X, Y, Z);
			OutModifiedPositions.push_front(FIntVector(X, Y, Z) + Position);

			ColorsDiffs.pop_front();
		}
	}
	else
	{
		uint64 Pow = IntPow9(Depth);
		if ((!ValuesDiffs.empty() && Id / Pow == ValuesDiffs.front().Id / Pow) || (!ColorsDiffs.empty() && Id / Pow == ColorsDiffs.front().Id / Pow))
		{
			if (IsLeaf())
			{
				bIsDirty = true;
				CreateChilds();
			}
			for (auto Child : Childs)
			{
				Child->LoadFromDiffListAndGetModifiedPositions(ValuesDiffs, ColorsDiffs, OutModifiedPositions);
			}
		}
	}
}


void ValueOctree::CreateChilds()
{
	check(IsLeaf());
	check(Childs.Num() == 0);
	check(Depth != 0);

	int d = Size() / 4;
	uint64 Pow = IntPow9(Depth - 1);

	Childs.Add(new ValueOctree(WorldGenerator, Position + FIntVector(-d, -d, -d), Depth - 1, Id + 1 * Pow));
	Childs.Add(new ValueOctree(WorldGenerator, Position + FIntVector(+d, -d, -d), Depth - 1, Id + 2 * Pow));
	Childs.Add(new ValueOctree(WorldGenerator, Position + FIntVector(-d, +d, -d), Depth - 1, Id + 3 * Pow));
	Childs.Add(new ValueOctree(WorldGenerator, Position + FIntVector(+d, +d, -d), Depth - 1, Id + 4 * Pow));
	Childs.Add(new ValueOctree(WorldGenerator, Position + FIntVector(-d, -d, +d), Depth - 1, Id + 5 * Pow));
	Childs.Add(new ValueOctree(WorldGenerator, Position + FIntVector(+d, -d, +d), Depth - 1, Id + 6 * Pow));
	Childs.Add(new ValueOctree(WorldGenerator, Position + FIntVector(-d, +d, +d), Depth - 1, Id + 7 * Pow));
	Childs.Add(new ValueOctree(WorldGenerator, Position + FIntVector(+d, +d, +d), Depth - 1, Id + 8 * Pow));

	bHasChilds = true;
	check(!IsLeaf() == (Childs.Num() == 8));
}

void ValueOctree::SetAsDirty()
{
	check(!IsDirty());
	check(Depth == 0);

	Values.SetNumUninitialized(16 * 16 * 16);
	Colors.SetNumUninitialized(16 * 16 * 16);
	for (int X = 0; X < 16; X++)
	{
		for (int Y = 0; Y < 16; Y++)
		{
			for (int Z = 0; Z < 16; Z++)
			{
				int GlobalX, GlobalY, GlobalZ;
				LocalToGlobal(X, Y, Z, GlobalX, GlobalY, GlobalZ);

				float Value;
				FColor Color;
				GetValueAndColor(GlobalX, GlobalY, GlobalZ, Value, Color);
				Values[X + 16 * Y + 16 * 16 * Z] = Value;
				Colors[X + 16 * Y + 16 * 16 * Z] = Color;
			}
		}
	}
	bIsDirty = true;
}

FORCEINLINE int ValueOctree::IndexFromCoordinates(int X, int Y, int Z)
{
	return X + 16 * Y + 16 * 16 * Z;
}

FORCEINLINE void ValueOctree::CoordinatesFromIndex(int Index, int& OutX, int& OutY, int& OutZ)
{
	OutX = Index % 16;

	Index = (Index - OutX) / 16;
	OutY = Index % 16;

	Index = (Index - OutY) / 16;
	OutZ = Index;
}

ValueOctree * ValueOctree::GetChild(int X, int Y, int Z)
{
	check(!IsLeaf());
	// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
	return Childs[(X >= Position.X) + 2 * (Y >= Position.Y) + 4 * (Z >= Position.Z)];
}

ValueOctree* ValueOctree::GetLeaf(int X, int Y, int Z)
{
	check(IsInOctree(X, Y, Z));

	if (IsLeaf())
	{
		return this;
	}
	else
	{
		return GetChild(X, Y, Z)->GetLeaf(X, Y, Z);
	}
}

void ValueOctree::GetDirtyChunksPositions(std::forward_list<FIntVector>& OutPositions)
{
	if (IsDirty())
	{
		if (IsLeaf())
		{
			// With neighbors
			const int S = Size();
			OutPositions.push_front(Position - FIntVector(0, 0, 0));
			OutPositions.push_front(Position - FIntVector(S, 0, 0));
			OutPositions.push_front(Position - FIntVector(0, S, 0));
			OutPositions.push_front(Position - FIntVector(S, S, 0));
			OutPositions.push_front(Position - FIntVector(0, 0, S));
			OutPositions.push_front(Position - FIntVector(S, 0, S));
			OutPositions.push_front(Position - FIntVector(0, S, S));
			OutPositions.push_front(Position - FIntVector(S, S, S));
		}
		else
		{
			for (auto Child : Childs)
			{
				Child->GetDirtyChunksPositions(OutPositions);
			}
		}
	}
}

//ValueOctree* ValueOctree::GetCopy()
//{
//	ValueOctree*  NewOctree = new ValueOctree(WorldGenerator, Position, Depth, Id);
//
//	if (Depth == 0)
//	{
//		TArray<float, TFixedAllocator<16 * 16 * 16>> ValuesCopy;
//		TArray<FColor, TFixedAllocator<16 * 16 * 16>> ColorsCopy;
//
//		if (IsDirty())
//		{
//			ValuesCopy.SetNumUninitialized(4096);
//			ColorsCopy.SetNumUninitialized(4096);
//
//			for (int X = 0; X < 16; X++)
//			{
//				for (int Y = 0; Y < 16; Y++)
//				{
//					for (int Z = 0; Z < 16; Z++)
//					{
//						ValuesCopy[X + 16 * Y + 16 * 16 * Z] = Values[X + 16 * Y + 16 * 16 * Z];
//						ColorsCopy[X + 16 * Y + 16 * 16 * Z] = Colors[X + 16 * Y + 16 * 16 * Z];
//					}
//				}
//			}
//		}
//
//		NewOctree->Values = ValuesCopy;
//		NewOctree->Colors = ColorsCopy;
//		NewOctree->bHasChilds = false;
//		NewOctree->bIsDirty = bIsDirty;
//	}
//	else if (IsLeaf())
//	{
//		NewOctree->bHasChilds = false;
//		check(!bIsDirty);
//		NewOctree->bIsDirty = false;
//	}
//	else
//	{
//		NewOctree->bHasChilds = true;
//		check(bIsDirty);
//		NewOctree->bIsDirty = true;
//		for (auto Child : Childs)
//		{
//			NewOctree->Childs.Add(Child->GetCopy());
//		}
//	}
//	return NewOctree;
//}
