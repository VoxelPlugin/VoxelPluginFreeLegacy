#include "VoxelPrivatePCH.h"
#include "ValueOctree.h"
#include "VoxelChunk.h"
#include "VoxelData.h"
#include "VoxelWorldGenerator.h"
#include "DrawDebugHelpers.h"

float ValueOctree::GetValue(FIntVector GlobalPosition)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(IsInOctree(GlobalPosition));
	check(!(IsDirty() && IsLeaf() && Depth != 0));

	if (IsDirty())
	{
		if (Depth == 0)
		{
			FIntVector P = GlobalToLocal(GlobalPosition);
			return Values[P.X + Width() * P.Y + Width() * Width() * P.Z];
		}
		else
		{
			return GetChild(GlobalPosition)->GetValue(GlobalPosition);
		}
	}
	else
	{
		if (WorldGenerator.GetObject()->IsValidLowLevel())
		{
			return IVoxelWorldGenerator::Execute_GetDefaultValue(WorldGenerator.GetObject(), GlobalPosition);
		}
		else
		{
			return 0;
		}
	}
}

FColor ValueOctree::GetColor(FIntVector GlobalPosition)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(IsInOctree(GlobalPosition));
	check(!(IsDirty() && IsLeaf() && Depth != 0));

	if (IsDirty())
	{
		if (Depth == 0)
		{
			FIntVector P = GlobalToLocal(GlobalPosition);
			return Colors[P.X + Width() * P.Y + Width() * Width() * P.Z];
		}
		else
		{
			return GetChild(GlobalPosition)->GetColor(GlobalPosition);
		}
	}
	else
	{
		if (WorldGenerator.GetObject()->IsValidLowLevel())
		{
			return IVoxelWorldGenerator::Execute_GetDefaultColor(WorldGenerator.GetObject(), GlobalPosition);
		}
		else
		{
			return FColor::Black;
		}
	}
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
			bIsDirty = true;
			CreateChilds();
			GetChild(GlobalPosition)->SetValue(GlobalPosition, Value);
		}
		else
		{
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
			bIsDirty = true;
			CreateChilds();
			GetChild(GlobalPosition)->SetColor(GlobalPosition, Color);
		}
		else
		{
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
				World->QueueUpdate(Position);
				World->QueueUpdate(Position - FIntVector(Width(), 0, 0));
				World->QueueUpdate(Position - FIntVector(0, Width(), 0));
				World->QueueUpdate(Position - FIntVector(Width(), Width(), 0));
				World->QueueUpdate(Position - FIntVector(0, 0, Width()));
				World->QueueUpdate(Position - FIntVector(Width(), 0, Width()));
				World->QueueUpdate(Position - FIntVector(0, Width(), Width()));
				World->QueueUpdate(Position - FIntVector(Width(), Width(), Width()));
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

				World->QueueUpdate(LocalToGlobal(FIntVector(X, Y, Z)));

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

				World->QueueUpdate(LocalToGlobal(FIntVector(X, Y, Z)));

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
	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int z = 0; z < 16; z++)
			{
				Values[x + 16 * y + 16 * 16 * z] = GetValue(LocalToGlobal(FIntVector(x, y, z)));
				Colors[x + 16 * y + 16 * 16 * z] = GetColor(LocalToGlobal(FIntVector(x, y, z)));
			}
		}
	}
	bIsDirty = true;
}

ValueOctree* ValueOctree::GetChild(FIntVector GlobalPosition)
{
	check(!IsLeaf());
	// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
	return Childs[(GlobalPosition.X >= Position.X ? 1 : 0) + (GlobalPosition.Y >= Position.Y ? 2 : 0) + (GlobalPosition.Z >= Position.Z ? 4 : 0)];
}

void ValueOctree::QueueUpdateOfDirtyChunks(AVoxelWorld* World)
{
	check(World);
	check(!(IsDirty() && IsLeaf() && Depth != 0));

	if (IsDirty())
	{
		if (IsLeaf())
		{
			World->QueueUpdate(Position);
			World->QueueUpdate(Position - FIntVector(Width(), 0, 0));
			World->QueueUpdate(Position - FIntVector(0, Width(), 0));
			World->QueueUpdate(Position - FIntVector(Width(), Width(), 0));
			World->QueueUpdate(Position - FIntVector(0, 0, Width()));
			World->QueueUpdate(Position - FIntVector(Width(), 0, Width()));
			World->QueueUpdate(Position - FIntVector(0, Width(), Width()));
			World->QueueUpdate(Position - FIntVector(Width(), Width(), Width()));
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
