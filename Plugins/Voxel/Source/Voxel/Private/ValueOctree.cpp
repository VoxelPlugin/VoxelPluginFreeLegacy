#include "VoxelPrivatePCH.h"
#include "ValueOctree.h"
#include "VoxelChunk.h"
#include "VoxelData.h"
#include "VoxelWorldGenerator.h"

ValueOctree::ValueOctree(FIntVector Position, int Depth, UVoxelWorldGenerator* WorldGenerator) : Position(Position), Depth(Depth), bIsDirty(false), bIsLeaf(true), WorldGenerator(WorldGenerator)
{
	check(WorldGenerator);
	check(Depth >= 0);
}




inline int ValueOctree::Width() const
{
	return 16 << Depth;
}

bool ValueOctree::IsLeaf() const
{
	return bIsLeaf;
}

bool ValueOctree::IsDirty() const
{
	return bIsDirty;
}

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
		return WorldGenerator->GetDefaultValue(GlobalPosition);
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
		return WorldGenerator->GetDefaultColor(GlobalPosition);
	}
}

void ValueOctree::SetValue(FIntVector GlobalPosition, float Value)
{
	check(!IsLeaf() == (Childs.Num() == 8));
	check(IsInOctree(GlobalPosition));
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
			FIntVector P = GlobalToLocal(GlobalPosition);
			Values[P.X + 16 * P.Y + 16 * 16 * P.Z] = Value;
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

bool ValueOctree::IsInOctree(FIntVector GlobalPosition) const
{
	FIntVector P = GlobalToLocal(GlobalPosition);
	return 0 <= P.X && 0 <= P.Y && 0 <= P.Z && P.X < Width() && P.Y < Width() && P.Z < Width();
}

FIntVector ValueOctree::GlobalToLocal(FIntVector GlobalPosition) const
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

FIntVector ValueOctree::LocalToGlobal(FIntVector LocalPosition) const
{
	return FIntVector(LocalPosition.X + (Position.X - Width() / 2), LocalPosition.Y + (Position.Y - Width() / 2), LocalPosition.Z + (Position.Z - Width() / 2));
}



void ValueOctree::CreateChilds()
{
	check(IsLeaf());
	check(Childs.Num() == 0);
	check(Depth != 0);
	int d = Width() / 4;
	Childs.Add(new ValueOctree(Position + FIntVector(-d, -d, -d), Depth - 1, WorldGenerator));
	Childs.Add(new ValueOctree(Position + FIntVector(+d, -d, -d), Depth - 1, WorldGenerator));
	Childs.Add(new ValueOctree(Position + FIntVector(-d, +d, -d), Depth - 1, WorldGenerator));
	Childs.Add(new ValueOctree(Position + FIntVector(+d, +d, -d), Depth - 1, WorldGenerator));
	Childs.Add(new ValueOctree(Position + FIntVector(-d, -d, +d), Depth - 1, WorldGenerator));
	Childs.Add(new ValueOctree(Position + FIntVector(+d, -d, +d), Depth - 1, WorldGenerator));
	Childs.Add(new ValueOctree(Position + FIntVector(-d, +d, +d), Depth - 1, WorldGenerator));
	Childs.Add(new ValueOctree(Position + FIntVector(+d, +d, +d), Depth - 1, WorldGenerator));
	bIsLeaf = false;
	check(!IsLeaf() == (Childs.Num() == 8));
}

ValueOctree* ValueOctree::GetChild(FIntVector GlobalPosition)
{
	check(!IsLeaf());
	// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
	return Childs[(GlobalPosition.X >= Position.X ? 1 : 0) + (GlobalPosition.Y >= Position.Y ? 2 : 0) + (GlobalPosition.Z >= Position.Z ? 4 : 0)];
}