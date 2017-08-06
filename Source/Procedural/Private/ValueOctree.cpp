#include "ValueOctree.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "VoxelData.h"

ValueOctree::ValueOctree(FIntVector Position, int Depth, VoxelData* Data) : Position(Position), Depth(Depth), bIsDirty(false), bIsLeaf(true), Data(Data)
{
	check(Data);
	check(Depth >= 0);
}




inline int ValueOctree::Width()
{
	return 16 << Depth;
}

bool ValueOctree::IsLeaf()
{
	return bIsLeaf;
}

bool ValueOctree::IsDirty()
{
	return bIsDirty;
}

signed char ValueOctree::GetValue(FIntVector GlobalPosition)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(IsInChunk(GlobalPosition));
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
		return Data->GetDefaultValue(GlobalPosition);
	}
}

FColor ValueOctree::GetColor(FIntVector GlobalPosition)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(IsInChunk(GlobalPosition));
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
		return Data->GetDefaultColor(GlobalPosition);
	}
}

void ValueOctree::SetValue(FIntVector GlobalPosition, signed char Value)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(IsInChunk(GlobalPosition));
	check(!(IsDirty() && IsLeaf() && Depth != 0));

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
				Values.SetNumUninitialized(Width() * Width() * Width());
				Colors.SetNumUninitialized(Width() * Width() * Width());
				for (int x = 0; x < Width(); x++)
				{
					for (int y = 0; y < Width(); y++)
					{
						for (int z = 0; z < Width(); z++)
						{
							Values[x + Width() * y + Width() * Width() * z] = GetValue(LocalToGlobal(FIntVector(x, y, z)));
							Colors[x + Width() * y + Width() * Width() * z] = GetColor(LocalToGlobal(FIntVector(x, y, z)));
						}
					}
				}
				bIsDirty = true;
			}
			FIntVector P = GlobalToLocal(GlobalPosition);
			Values[P.X + Width() * P.Y + Width() * Width() * P.Z] = Value;
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
	check(IsInChunk(GlobalPosition));
	check(!(IsDirty() && IsLeaf() && Depth != 0));

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
				Values.SetNumUninitialized(Width() * Width() * Width());
				Colors.SetNumUninitialized(Width() * Width() * Width());
				for (int x = 0; x < Width(); x++)
				{
					for (int y = 0; y < Width(); y++)
					{
						for (int z = 0; z < Width(); z++)
						{
							Values[x + Width() * y + Width() * Width() * z] = GetValue(LocalToGlobal(FIntVector(x, y, z)));
							Colors[x + Width() * y + Width() * Width() * z] = GetColor(LocalToGlobal(FIntVector(x, y, z)));
						}
					}
				}
				bIsDirty = true;
			}
			FIntVector P = GlobalToLocal(GlobalPosition);
			Colors[P.X + Width() * P.Y + Width() * Width() * P.Z] = Color;
		}
	}
	else
	{
		check(IsDirty());
		GetChild(GlobalPosition)->SetColor(GlobalPosition, Color);
	}
}

bool ValueOctree::IsInChunk(FIntVector GlobalPosition)
{
	FIntVector P = GlobalToLocal(GlobalPosition);
	return 0 <= P.X && 0 <= P.Y && 0 <= P.Z && P.X < Width() && P.Y < Width() && P.Z < Width();
}

FIntVector ValueOctree::GlobalToLocal(FIntVector GlobalPosition)
{
	return FIntVector(GlobalPosition.X - (Position.X - Width() / 2), GlobalPosition.Y - (Position.Y - Width() / 2), GlobalPosition.Z - (Position.Z - Width() / 2));
}

void ValueOctree::AddChunksToArray(TArray<FVoxelChunkSaveStruct> SaveArray)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(!(IsDirty() && IsLeaf() && Depth != 0));

	if (IsDirty())
	{
		if (IsLeaf())
		{
			FVoxelChunkSaveStruct SaveStruct(Position, Depth, Values, Colors);
			SaveArray.Add(SaveStruct);
		}
		else
		{
			for (int i = 0; i < 8; i++)
			{
				Childs[i]->AddChunksToArray(SaveArray);
			}
		}
	}
}

void ValueOctree::LoadFromArray(TArray<FVoxelChunkSaveStruct> SaveArray)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(!(IsDirty() && IsLeaf() && Depth != 0));

	if (IsLeaf())
	{
		for (auto SaveStruct : SaveArray)
		{
			if (SaveStruct.Position == Position && SaveStruct.Depth == Depth)
			{
				Values.SetNumUninitialized(SaveStruct.Values.Num());
				for (int i = 0; i < Values.Num(); i++)
				{
					Values[i] = FMath::Clamp(SaveStruct.Values[i], -127, 127);
				}
				Colors = SaveStruct.Colors;
			}
		}
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			Childs[i]->LoadFromArray(SaveArray);
		}
	}
}

FIntVector ValueOctree::LocalToGlobal(FIntVector LocalPosition)
{
	return FIntVector(LocalPosition.X + (Position.X - Width() / 2), LocalPosition.Y + (Position.Y - Width() / 2), LocalPosition.Z + (Position.Z - Width() / 2));
}



void ValueOctree::CreateChilds()
{
	check(IsLeaf());
	check(Childs.Num() == 0);
	check(Depth != 0);
	int d = Width() / 4;
	Childs.Add(new ValueOctree(Position + FIntVector(-d, -d, -d), Depth - 1, Data));
	Childs.Add(new ValueOctree(Position + FIntVector(+d, -d, -d), Depth - 1, Data));
	Childs.Add(new ValueOctree(Position + FIntVector(-d, +d, -d), Depth - 1, Data));
	Childs.Add(new ValueOctree(Position + FIntVector(+d, +d, -d), Depth - 1, Data));
	Childs.Add(new ValueOctree(Position + FIntVector(-d, -d, +d), Depth - 1, Data));
	Childs.Add(new ValueOctree(Position + FIntVector(+d, -d, +d), Depth - 1, Data));
	Childs.Add(new ValueOctree(Position + FIntVector(-d, +d, +d), Depth - 1, Data));
	Childs.Add(new ValueOctree(Position + FIntVector(+d, +d, +d), Depth - 1, Data));
	bIsLeaf = false;
	check(!IsLeaf() == (Childs.Num() == 8));
}

ValueOctree* ValueOctree::GetChild(FIntVector GlobalPosition)
{
	check(!IsLeaf());
	// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
	return Childs[(GlobalPosition.X >= Position.X ? 1 : 0) + (GlobalPosition.Y >= Position.Y ? 2 : 0) + (GlobalPosition.Z >= Position.Z ? 4 : 0)];
}