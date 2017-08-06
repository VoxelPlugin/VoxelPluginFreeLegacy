#include "ValueOctree.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "VoxelData.h"

ValueOctree::ValueOctree(FIntVector Position, int Depth, VoxelData* Data) : Position(Position), Depth(Depth), bIsDirty(false), bIsLeaf(true), Data(Data)
{

}




inline int ValueOctree::GetWidth()
{
	return 16 << Depth;
}

void ValueOctree::CreateTree()
{
	check(!IsLeaf() == (Childs.Num() == 8));
	if (Depth != 0)
	{
		CreateChilds();
		for (int i = 0; i < 8; i++)
		{
			Childs[i]->CreateTree();
		}
	}
}

ValueOctree* ValueOctree::GetLeaf(FIntVector GlobalPosition)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(IsInChunk(GlobalPosition));
	if (IsLeaf())
	{
		return this;
	}
	else
	{
		// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
		return Childs[(GlobalPosition.X >= Position.X ? 1 : 0) + (GlobalPosition.Y >= Position.Y ? 2 : 0) + (GlobalPosition.Z >= Position.Z ? 4 : 0)]->GetLeaf(GlobalPosition);
	};
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

	if (IsInChunk(GlobalPosition))
	{
		if (IsLeaf())
		{
			if (IsDirty())
			{
				int w = GetWidth();
				FIntVector P = GlobalToLocal(GlobalPosition);
				return Values[P.X + w * P.Y + w * w * P.Z];
			}
			else
			{
				return Data->GetDefaultValue(GlobalPosition);
			}
		}
		else
		{
			return GetLeaf(GlobalPosition)->GetValue(GlobalPosition);
		}
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Get value error: (%d, %d, %d) not in chunk (%d-%d, %d-%d, %d-%d)"),
			GlobalPosition.X, GlobalPosition.Y, GlobalPosition.Z,
			Position.X - GetWidth() / 2, Position.X + GetWidth() / 2,
			Position.Y - GetWidth() / 2, Position.Y + GetWidth() / 2,
			Position.Z - GetWidth() / 2, Position.Z + GetWidth() / 2);
		return (GlobalPosition.Z > 10) ? 1 : -1;
	}
}

FColor ValueOctree::GetColor(FIntVector GlobalPosition)
{
	check(!IsLeaf() == (Childs.Num() == 8));

	if (IsInChunk(GlobalPosition))
	{
		if (IsLeaf())
		{
			if (IsDirty())
			{
				int w = GetWidth();
				FIntVector P = GlobalToLocal(GlobalPosition);
				return Colors[P.X + w * P.Y + w * w * P.Z];
			}
			else
			{
				return Data->GetDefaultColor(GlobalPosition);
			}
		}
		else
		{
			return GetLeaf(GlobalPosition)->GetColor(GlobalPosition);
		}
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Get color error: (%d, %d, %d) not in chunk (%d-%d, %d-%d, %d-%d)"),
			GlobalPosition.X, GlobalPosition.Y, GlobalPosition.Z,
			Position.X - GetWidth() / 2, Position.X + GetWidth() / 2,
			Position.Y - GetWidth() / 2, Position.Y + GetWidth() / 2,
			Position.Z - GetWidth() / 2, Position.Z + GetWidth() / 2);
		return FColor::Red;
	}
}

void ValueOctree::SetValue(FIntVector GlobalPosition, signed char Value)
{
	check(!IsLeaf() == (Childs.Num() == 8));

	if (IsInChunk(GlobalPosition))
	{
		if (IsLeaf())
		{
			int w = GetWidth();
			if (!IsDirty())
			{
				Values.SetNumUninitialized(w * w * w);
				Colors.SetNumUninitialized(w * w * w);
				for (int i = 0; i < w; i++)
				{
					for (int j = 0; j < w; j++)
					{
						for (int k = 0; k < w; k++)
						{
							Values[i + w * j + w * w * k] = GetValue(LocalToGlobal(FIntVector(i, j, k)));
							Colors[i + w * j + w * w * k] = GetColor(LocalToGlobal(FIntVector(i, j, k)));
						}
					}
				}
				bIsDirty = true;
			}
			FIntVector P = GlobalToLocal(GlobalPosition);
			Values[P.X + w * P.Y + w * w * P.Z] = Value;
		}
		else
		{
			GetLeaf(GlobalPosition)->SetValue(GlobalPosition, Value);
		}
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Set value error: (%d, %d, %d) not in chunk (%d-%d, %d-%d, %d-%d)"),
			GlobalPosition.X, GlobalPosition.Y, GlobalPosition.Z,
			Position.X - GetWidth() / 2, Position.X + GetWidth() / 2,
			Position.Y - GetWidth() / 2, Position.Y + GetWidth() / 2,
			Position.Z - GetWidth() / 2, Position.Z + GetWidth() / 2);
	}
}

void ValueOctree::SetColor(FIntVector GlobalPosition, FColor Color)
{
	check(!IsLeaf() == (Childs.Num() == 8));

	if (IsInChunk(GlobalPosition))
	{
		if (IsLeaf())
		{
			int w = GetWidth();
			if (!IsDirty())
			{
				Values.SetNumUninitialized(w * w * w);
				Colors.SetNumUninitialized(w * w * w);
				for (int i = 0; i < w; i++)
				{
					for (int j = 0; j < w; j++)
					{
						for (int k = 0; k < w; k++)
						{
							Values[i + w * j + w * w * k] = GetValue(LocalToGlobal(FIntVector(i, j, k)));
							Colors[i + w * j + w * w * k] = GetColor(LocalToGlobal(FIntVector(i, j, k)));
						}
					}
				}
				bIsDirty = true;
			}
			FIntVector P = GlobalToLocal(GlobalPosition);
			Colors[P.X + w * P.Y + w * w * P.Z] = Color;
		}
		else
		{
			GetLeaf(GlobalPosition)->SetColor(GlobalPosition, Color);
		}
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Set color error: (%d, %d, %d) not in chunk (%d-%d, %d-%d, %d-%d)"),
			GlobalPosition.X, GlobalPosition.Y, GlobalPosition.Z,
			Position.X - GetWidth() / 2, Position.X + GetWidth() / 2,
			Position.Y - GetWidth() / 2, Position.Y + GetWidth() / 2,
			Position.Z - GetWidth() / 2, Position.Z + GetWidth() / 2);
	}
}

bool ValueOctree::IsInChunk(FIntVector GlobalPosition)
{
	FIntVector P = GlobalToLocal(GlobalPosition);
	return 0 <= P.X && 0 <= P.Y && 0 <= P.Z && P.X < GetWidth() && P.Y < GetWidth() && P.Z < GetWidth();
}

FIntVector ValueOctree::GlobalToLocal(FIntVector GlobalPosition)
{
	return FIntVector(GlobalPosition.X - (Position.X - GetWidth() / 2), GlobalPosition.Y - (Position.Y - GetWidth() / 2), GlobalPosition.Z - (Position.Z - GetWidth() / 2));
}

void ValueOctree::AddChunksToArray(TArray<FVoxelChunkSaveStruct> SaveArray)
{
	check(!IsLeaf() == (Childs.Num() == 8));

	if (IsLeaf())
	{
		if (IsDirty())
		{
			FVoxelChunkSaveStruct SaveStruct(Position, Depth, Values, Colors);
			SaveArray.Add(SaveStruct);
		}
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			Childs[i]->AddChunksToArray(SaveArray);
		}
	}
}

void ValueOctree::LoadFromArray(TArray<FVoxelChunkSaveStruct> SaveArray)
{
	check(!IsLeaf() == (Childs.Num() == 8));

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
	return FIntVector(LocalPosition.X + (Position.X - GetWidth() / 2), LocalPosition.Y + (Position.Y - GetWidth() / 2), LocalPosition.Z + (Position.Z - GetWidth() / 2));
}



void ValueOctree::CreateChilds()
{
	check(IsLeaf());
	check(Childs.Num() == 0);
	int d = GetWidth() / 4;
	Childs.Add(TSharedRef<ValueOctree>(new ValueOctree(Position + FIntVector(-d, -d, -d), Depth - 1, Data)));
	Childs.Add(TSharedRef<ValueOctree>(new ValueOctree(Position + FIntVector(+d, -d, -d), Depth - 1, Data)));
	Childs.Add(TSharedRef<ValueOctree>(new ValueOctree(Position + FIntVector(-d, +d, -d), Depth - 1, Data)));
	Childs.Add(TSharedRef<ValueOctree>(new ValueOctree(Position + FIntVector(+d, +d, -d), Depth - 1, Data)));
	Childs.Add(TSharedRef<ValueOctree>(new ValueOctree(Position + FIntVector(-d, -d, +d), Depth - 1, Data)));
	Childs.Add(TSharedRef<ValueOctree>(new ValueOctree(Position + FIntVector(+d, -d, +d), Depth - 1, Data)));
	Childs.Add(TSharedRef<ValueOctree>(new ValueOctree(Position + FIntVector(-d, +d, +d), Depth - 1, Data)));
	Childs.Add(TSharedRef<ValueOctree>(new ValueOctree(Position + FIntVector(+d, +d, +d), Depth - 1, Data)));
	bIsLeaf = false;
	check(!IsLeaf() == (Childs.Num() == 8));
}