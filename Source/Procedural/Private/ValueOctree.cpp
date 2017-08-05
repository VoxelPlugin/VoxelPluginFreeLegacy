#include "ValueOctree.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "VoxelData.h"
#include "EngineGlobals.h"
#include "Engine.h"

DEFINE_LOG_CATEGORY(ValueOctreeLog);

ValueOctree::ValueOctree(FIntVector position, int depth, VoxelData* data) : Position(position), Depth(depth), bIsDirty(false), bIsLeaf(true), Data(data)
{

}

ValueOctree::~ValueOctree()
{
	for (int i = 0; i < 8; i++)
	{
		Childs[i].Reset();
	}
}




inline int ValueOctree::GetWidth()
{
	return 16 << Depth;
}

void ValueOctree::CreateTree()
{
	if (Depth != 0)
	{
		CreateChilds();
		for (int i = 0; i < 8; i++)
		{
			Childs[i]->CreateTree();
		}
	}
}

ValueOctree* ValueOctree::GetLeaf(FIntVector position)
{
	if (IsLeaf())
	{
		check(IsInChunk(position));
		return this;
	}
	else
	{
		// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
		int d = GetWidth() / 2;
		return Childs[(position.X >= Position.X ? 1 : 0) + (position.Y >= Position.Y ? 2 : 0) + (position.Z >= Position.Z ? 4 : 0)]->GetLeaf(position);
	}
}

bool ValueOctree::IsLeaf()
{
	return bIsLeaf;
}

bool ValueOctree::IsDirty()
{
	return bIsDirty;
}

signed char ValueOctree::GetValue(FIntVector globalPosition)
{
	if (IsInChunk(globalPosition))
	{
		if (IsLeaf())
		{
			if (IsDirty())
			{
				int w = GetWidth();
				FIntVector P = GlobalToLocal(globalPosition);
				return Values[P.X + w * P.Y + w * w * P.Z];
			}
			else
			{
				return Data->GetDefaultValue(globalPosition);
			}
		}
		else
		{
			return GetLeaf(globalPosition)->GetValue(globalPosition);
		}
	}
	else
	{
		UE_LOG(ValueOctreeLog, Error, TEXT("Get value error: (%d, %d, %d) not in chunk (%d-%d, %d-%d, %d-%d)"),
			globalPosition.X, globalPosition.Y, globalPosition.Z,
			Position.X - GetWidth() / 2, Position.X + GetWidth() / 2,
			Position.Y - GetWidth() / 2, Position.Y + GetWidth() / 2,
			Position.Z - GetWidth() / 2, Position.Z + GetWidth() / 2);
		return (globalPosition.Z > 10) ? 1 : -1;
	}
}

FColor ValueOctree::GetColor(FIntVector globalPosition)
{
	if (IsInChunk(globalPosition))
	{
		if (IsLeaf())
		{
			if (IsDirty())
			{
				int w = GetWidth();
				FIntVector P = GlobalToLocal(globalPosition);
				return Colors[P.X + w * P.Y + w * w * P.Z];
			}
			else
			{
				return Data->GetDefaultColor(globalPosition);
			}
		}
		else
		{
			return GetLeaf(globalPosition)->GetColor(globalPosition);
		}
	}
	else
	{
		UE_LOG(ValueOctreeLog, Error, TEXT("Get color error: (%d, %d, %d) not in chunk (%d-%d, %d-%d, %d-%d)"),
			globalPosition.X, globalPosition.Y, globalPosition.Z,
			Position.X - GetWidth() / 2, Position.X + GetWidth() / 2,
			Position.Y - GetWidth() / 2, Position.Y + GetWidth() / 2,
			Position.Z - GetWidth() / 2, Position.Z + GetWidth() / 2);
		return FColor::Red;
	}
}

void ValueOctree::SetValue(FIntVector globalPosition, signed char value)
{
	if (IsInChunk(globalPosition))
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
			FIntVector P = GlobalToLocal(globalPosition);
			Values[P.X + w * P.Y + w * w * P.Z] = value;
		}
		else
		{
			GetLeaf(globalPosition)->SetValue(globalPosition, value);
		}
	}
	else
	{
		UE_LOG(ValueOctreeLog, Error, TEXT("Set value error: (%d, %d, %d) not in chunk (%d-%d, %d-%d, %d-%d)"),
			globalPosition.X, globalPosition.Y, globalPosition.Z,
			Position.X - GetWidth() / 2, Position.X + GetWidth() / 2,
			Position.Y - GetWidth() / 2, Position.Y + GetWidth() / 2,
			Position.Z - GetWidth() / 2, Position.Z + GetWidth() / 2);
	}
}

void ValueOctree::SetColor(FIntVector globalPosition, FColor color)
{
	if (IsInChunk(globalPosition))
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
			FIntVector P = GlobalToLocal(globalPosition);
			Colors[P.X + w * P.Y + w * w * P.Z] = color;
		}
		else
		{
			GetLeaf(globalPosition)->SetColor(globalPosition, color);
		}
	}
	else
	{
		UE_LOG(ValueOctreeLog, Error, TEXT("Set color error: (%d, %d, %d) not in chunk (%d-%d, %d-%d, %d-%d)"),
			globalPosition.X, globalPosition.Y, globalPosition.Z,
			Position.X - GetWidth() / 2, Position.X + GetWidth() / 2,
			Position.Y - GetWidth() / 2, Position.Y + GetWidth() / 2,
			Position.Z - GetWidth() / 2, Position.Z + GetWidth() / 2);
	}
}

bool ValueOctree::IsInChunk(FIntVector globalPosition)
{
	FIntVector P = GlobalToLocal(globalPosition);
	return 0 <= P.X && 0 <= P.Y && 0 <= P.Z && P.X < GetWidth() && P.Y < GetWidth() && P.Z < GetWidth();
}

FIntVector ValueOctree::GlobalToLocal(FIntVector globalPosition)
{
	return FIntVector(globalPosition.X - (Position.X - GetWidth() / 2), globalPosition.Y - (Position.Y - GetWidth() / 2), globalPosition.Z - (Position.Z - GetWidth() / 2));
}

void ValueOctree::AddChunksToArray(TArray<FVoxelChunkSaveStruct> saveArray)
{
	if (IsLeaf())
	{
		if (IsDirty())
		{
			FVoxelChunkSaveStruct saveStruct(Position, Values, Colors);
		}
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			if (Childs[i].IsValid())
			{
				Childs[i]->AddChunksToArray(saveArray);
			}
		}
	}
}

void ValueOctree::LoadFromArray(TArray<FVoxelChunkSaveStruct> saveArray)
{
	if (IsLeaf())
	{
		for (auto saveStruct : saveArray)
		{
			if (saveStruct.Position == Position)
			{
				Values.SetNumUninitialized(saveStruct.Values.Num());
				for (int i = 0; i < Values.Num(); i++)
				{
					Values[i] = FMath::Clamp(saveStruct.Values[i], -127, 127);
				}
				Colors = saveStruct.Colors;
			}
		}
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			if (Childs[i].IsValid())
			{
				Childs[i]->LoadFromArray(saveArray);
			}
		}
	}
}

FIntVector ValueOctree::LocalToGlobal(FIntVector localPosition)
{
	return FIntVector(localPosition.X + (Position.X - GetWidth() / 2), localPosition.Y + (Position.Y - GetWidth() / 2), localPosition.Z + (Position.Z - GetWidth() / 2));
}



void ValueOctree::CreateChilds()
{
	check(IsLeaf());
	int d = GetWidth() / 4;
	Childs[0] = MakeShareable(new ValueOctree(Position + FIntVector(-d, -d, -d), Depth - 1, Data));
	Childs[1] = MakeShareable(new ValueOctree(Position + FIntVector(+d, -d, -d), Depth - 1, Data));
	Childs[2] = MakeShareable(new ValueOctree(Position + FIntVector(-d, +d, -d), Depth - 1, Data));
	Childs[3] = MakeShareable(new ValueOctree(Position + FIntVector(+d, +d, -d), Depth - 1, Data));
	Childs[4] = MakeShareable(new ValueOctree(Position + FIntVector(-d, -d, +d), Depth - 1, Data));
	Childs[5] = MakeShareable(new ValueOctree(Position + FIntVector(+d, -d, +d), Depth - 1, Data));
	Childs[6] = MakeShareable(new ValueOctree(Position + FIntVector(-d, +d, +d), Depth - 1, Data));
	Childs[7] = MakeShareable(new ValueOctree(Position + FIntVector(+d, +d, +d), Depth - 1, Data));
	bIsLeaf = false;
}