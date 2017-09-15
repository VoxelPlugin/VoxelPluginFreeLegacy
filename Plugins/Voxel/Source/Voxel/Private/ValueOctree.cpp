#include "VoxelPrivatePCH.h"
#include "ValueOctree.h"
#include "VoxelChunk.h"
#include "VoxelData.h"
#include "VoxelWorldGenerator.h"
#include "DrawDebugHelpers.h"

ValueOctree::ValueOctree(bool bMultiplayer, AVoxelWorldGenerator* WorldGenerator, FIntVector Position, int Depth, int Id) : Octree(Position, Depth, Id), bMultiplayer(bMultiplayer), WorldGenerator(WorldGenerator), bIsDirty(false), bIsNetworkDirty(false)
{
}

FORCEINLINE bool ValueOctree::IsDirty() const
{
	return bIsDirty;
}

void ValueOctree::GetValueAndColor(int X, int Y, int Z, float& OutValue, FColor& OutColor)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(IsInOctree(X, Y, Z));
	check(!(IsDirty() && IsLeaf() && Depth != 0));

	if (UNLIKELY(IsDirty()))
	{
		if (UNLIKELY(Depth == 0))
		{
			FScopeLock Lock(&CriticalSection);
			int LocalX, LocalY, LocalZ;
			GlobalToLocal(X, Y, Z, LocalX, LocalY, LocalZ);
			int Index = LocalX + Width() * LocalY + Width() * Width() * LocalZ;
			OutValue = Values[Index];
			OutColor = Colors[Index];
		}
		else
		{
			return GetChild(X, Y, Z)->GetValueAndColor(X, Y, Z, OutValue, OutColor);
		}
	}
	else
	{
		OutValue = WorldGenerator->GetDefaultValue(X, Y, Z);
		OutColor = WorldGenerator->GetDefaultColor(X, Y, Z);
	}
}

float ValueOctree::GetValue(FIntVector GlobalPosition)
{
	float Value;
	FColor Color;
	GetValueAndColor(GlobalPosition.X, GlobalPosition.Y, GlobalPosition.Z, Value, Color);
	return Value;
}

void ValueOctree::SetValue(FIntVector GlobalPosition, float Value)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(IsInOctree(GlobalPosition));
	check(!(IsDirty() && IsLeaf() && Depth != 0));

	bIsNetworkDirty = true;

	if (IsLeaf())
	{
		if (Depth != 0)
		{
			CreateChilds();
			bIsDirty = true; // IsDirty only when having childs (for multithreading)
			GetChild(GlobalPosition)->SetValue(GlobalPosition, Value);
		}
		else
		{
			FScopeLock Lock(&CriticalSection);

			if (!IsDirty())
			{
				SetAsDirty();
			}

			FIntVector P = GlobalToLocal(GlobalPosition);
			int Index = P.X + 16 * P.Y + 16 * 16 * P.Z;
			Values[Index] = Value;

			if (bMultiplayer)
			{
				DirtyValues.Add(Index);
			}
		}
	}
	else
	{
		check(IsDirty());
		GetChild(GlobalPosition)->SetValue(GlobalPosition, Value);
	}
}

void ValueOctree::SetColor(FIntVector GlobalPosition, FColor Color)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(IsInOctree(GlobalPosition));
	check(!(IsDirty() && IsLeaf() && Depth != 0));

	bIsNetworkDirty = true;

	if (IsLeaf())
	{
		if (Depth != 0)
		{
			CreateChilds();
			bIsDirty = true; // IsDirty only when having childs (for multithreading)
			GetChild(GlobalPosition)->SetColor(GlobalPosition, Color);
		}
		else
		{
			FScopeLock Lock(&CriticalSection);

			if (!IsDirty())
			{
				SetAsDirty();
			}

			FIntVector P = GlobalToLocal(GlobalPosition);
			int Index = P.X + 16 * P.Y + 16 * 16 * P.Z;
			Colors[Index] = Color;

			if (bMultiplayer)
			{
				DirtyColors.Add(Index);
			}
		}
	}
	else
	{
		check(IsDirty());
		GetChild(GlobalPosition)->SetColor(GlobalPosition, Color);
	}
}

void ValueOctree::AddChunksToArray(std::list<FVoxelChunkSave>& SaveArray)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(!(IsDirty() && IsLeaf() && Depth != 0));

	if (IsDirty())
	{
		if (IsLeaf())
		{
			FVoxelChunkSave* SaveStruct = new FVoxelChunkSave(Id, Position, Values, Colors);
			SaveArray.push_back(*SaveStruct);
		}
		else
		{
			for (auto Child : Childs)
			{
				Child->AddChunksToArray(SaveArray);
			}
		}
	}
}

void ValueOctree::LoadAndQueueUpdateFromSave(std::list<FVoxelChunkSave>& SaveArray, AVoxelWorld* World)
{
	check(World);
	check(!IsLeaf() == (Childs.Num() == 8));
	check(!(IsDirty() && IsLeaf() && Depth != 0));

	if (SaveArray.empty())
	{
		return;
	}

	if (IsLeaf())
	{
		if (Depth == 0)
		{
			if (SaveArray.front().Id == Id)
			{
				bIsDirty = true;
				Values = SaveArray.front().Values;
				Colors = SaveArray.front().Colors;
				check(Values.Num() == 4096);
				check(Colors.Num() == 4096);
				SaveArray.pop_front();
				World->QueueUpdate(Position, false);
				World->QueueUpdate(Position - FIntVector(Width(), 0, 0), false);
				World->QueueUpdate(Position - FIntVector(0, Width(), 0), false);
				World->QueueUpdate(Position - FIntVector(Width(), Width(), 0), false);
				World->QueueUpdate(Position - FIntVector(0, 0, Width()), false);
				World->QueueUpdate(Position - FIntVector(Width(), 0, Width()), false);
				World->QueueUpdate(Position - FIntVector(0, Width(), Width()), false);
				World->QueueUpdate(Position - FIntVector(Width(), Width(), Width()), false);
			}
		}
		else
		{
			uint32 Pow = IntPow9(Depth);
			if (Id / Pow == SaveArray.front().Id / Pow)
			{
				bIsDirty = true;
				CreateChilds();
				for (auto Child : Childs)
				{
					Child->LoadAndQueueUpdateFromSave(SaveArray, World);
				}
			}
		}
	}
	else
	{
		uint32 Pow = IntPow9(Depth);
		if (Id / Pow == SaveArray.front().Id / Pow)
		{
			for (auto Child : Childs)
			{
				Child->LoadAndQueueUpdateFromSave(SaveArray, World);
			}
		}
	}
}

void ValueOctree::AddChunksToDiffArrays(VoxelValueDiffArray& ValuesDiffs, VoxelColorDiffArray& ColorsDiffs)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(!(IsDirty() && IsLeaf() && Depth != 0));

	if (bIsNetworkDirty)
	{
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
		bIsNetworkDirty = false;
	}
}

void ValueOctree::LoadAndQueueUpdateFromDiffArrays(std::forward_list<FVoxelValueDiff>& ValuesDiffs, std::forward_list<FVoxelColorDiff>& ColorsDiffs, AVoxelWorld* World)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(!(IsDirty() && IsLeaf() && Depth != 0));
	check(World);

	if (ValuesDiffs.empty() && ColorsDiffs.empty())
	{
		return;
	}

	if (IsLeaf())
	{
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


				int Index = ValuesDiffs.front().Index;
				int X = Index % 16;
				Index = (Index - X) / 16;
				int Y = Index % 16;
				Index = (Index - Y) / 16;
				int Z = Index;

				World->QueueUpdate(LocalToGlobal(FIntVector(X, Y, Z)), false);

				if (World->bDebugMultiplayer)
				{
					DrawDebugPoint(World->GetWorld(), World->GetTransform().TransformPosition((FVector)LocalToGlobal(FIntVector(X, Y, Z))), 5, FColor::Magenta, false, 1);
				}
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


				int Index = ColorsDiffs.front().Index;
				int X = Index % 16;
				Index = (Index - X) / 16;
				int Y = Index % 16;
				Index = (Index - Y) / 16;
				int Z = Index;

				World->QueueUpdate(LocalToGlobal(FIntVector(X, Y, Z)), false);

				if (World->bDebugMultiplayer)
				{
					DrawDebugPoint(World->GetWorld(), World->GetTransform().TransformPosition((FVector)LocalToGlobal(FIntVector(X, Y, Z))), 5, FColor::Magenta, false, 1);
				}

				ColorsDiffs.pop_front();
			}
		}
		else
		{
			uint32 Pow = IntPow9(Depth);
			if ((!ValuesDiffs.empty() && Id / Pow == ValuesDiffs.front().Id / Pow) || (!ColorsDiffs.empty() && Id / Pow == ColorsDiffs.front().Id / Pow))
			{
				bIsDirty = true;
				CreateChilds();
				for (auto Child : Childs)
				{
					Child->LoadAndQueueUpdateFromDiffArrays(ValuesDiffs, ColorsDiffs, World);
				}
			}
		}
	}
	else
	{
		uint32 Pow = IntPow9(Depth);
		if ((!ValuesDiffs.empty() && Id / Pow == ValuesDiffs.front().Id / Pow) || (!ColorsDiffs.empty() && Id / Pow == ColorsDiffs.front().Id / Pow))
		{
			for (auto Child : Childs)
			{
				Child->LoadAndQueueUpdateFromDiffArrays(ValuesDiffs, ColorsDiffs, World);
			}
		}
	}
}


void ValueOctree::CreateChilds()
{
	check(IsLeaf());
	check(Childs.Num() == 0);
	check(Depth != 0);

	int d = Width() / 4;
	uint32 Pow = IntPow9(Depth - 1);

	Childs.Add(new ValueOctree(bMultiplayer, WorldGenerator, Position + FIntVector(-d, -d, -d), Depth - 1, Id + 1 * Pow));
	Childs.Add(new ValueOctree(bMultiplayer, WorldGenerator, Position + FIntVector(+d, -d, -d), Depth - 1, Id + 2 * Pow));
	Childs.Add(new ValueOctree(bMultiplayer, WorldGenerator, Position + FIntVector(-d, +d, -d), Depth - 1, Id + 3 * Pow));
	Childs.Add(new ValueOctree(bMultiplayer, WorldGenerator, Position + FIntVector(+d, +d, -d), Depth - 1, Id + 4 * Pow));
	Childs.Add(new ValueOctree(bMultiplayer, WorldGenerator, Position + FIntVector(-d, -d, +d), Depth - 1, Id + 5 * Pow));
	Childs.Add(new ValueOctree(bMultiplayer, WorldGenerator, Position + FIntVector(+d, -d, +d), Depth - 1, Id + 6 * Pow));
	Childs.Add(new ValueOctree(bMultiplayer, WorldGenerator, Position + FIntVector(-d, +d, +d), Depth - 1, Id + 7 * Pow));
	Childs.Add(new ValueOctree(bMultiplayer, WorldGenerator, Position + FIntVector(+d, +d, +d), Depth - 1, Id + 8 * Pow));

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

ValueOctree* ValueOctree::GetChild(FIntVector GlobalPosition)
{
	return GetChild(GlobalPosition.X, GlobalPosition.Y, GlobalPosition.Z);
}

ValueOctree * ValueOctree::GetChild(int X, int Y, int Z)
{
	check(!IsLeaf());
	// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
	return Childs[(X >= Position.X) + 2 * (Y >= Position.Y) + 4 * (Z >= Position.Z)];
}

void ValueOctree::QueueUpdateOfDirtyChunks(AVoxelWorld* World)
{
	check(World);
	check(!(IsDirty() && IsLeaf() && Depth != 0));

	if (IsDirty())
	{
		if (IsLeaf())
		{
			World->QueueUpdate(Position, false);
			World->QueueUpdate(Position - FIntVector(Width(), 0, 0), false);
			World->QueueUpdate(Position - FIntVector(0, Width(), 0), false);
			World->QueueUpdate(Position - FIntVector(Width(), Width(), 0), false);
			World->QueueUpdate(Position - FIntVector(0, 0, Width()), false);
			World->QueueUpdate(Position - FIntVector(Width(), 0, Width()), false);
			World->QueueUpdate(Position - FIntVector(0, Width(), Width()), false);
			World->QueueUpdate(Position - FIntVector(Width(), Width(), Width()), false);
		}
		else
		{
			for (auto Child : Childs)
			{
				Child->QueueUpdateOfDirtyChunks(World);
			}
		}
	}
}
