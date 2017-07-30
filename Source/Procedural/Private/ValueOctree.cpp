#include "ValueOctree.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "EngineGlobals.h"
#include "Engine.h"

DEFINE_LOG_CATEGORY(ValueOctreeLog);

ValueOctree::ValueOctree(FIntVector position, int depth) : Position(position), Depth(depth), bIsDirty(false), bIsLeaf(true)
{

}

ValueOctree::~ValueOctree()
{

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
				return (globalPosition.Z == 8) ? 0 : ((globalPosition.Z > 8) ? 100 : -100);
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

void ValueOctree::SetValue(FIntVector globalPosition, signed char value)
{
	if (IsInChunk(globalPosition))
	{
		if (IsLeaf())
		{
			int w = GetWidth();
			if (!IsDirty())
			{
				Values = std::vector<signed char>(w * w * w);
				for (int i = 0; i < w; i++)
				{
					for (int j = 0; j < w; j++)
					{
						for (int k = 0; k < w; k++)
						{
							Values[i + w * j + w * w * k] = GetValue(LocalToGlobal(FIntVector(i, j, k)));
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

bool ValueOctree::IsInChunk(FIntVector globalPosition)
{
	FIntVector P = GlobalToLocal(globalPosition);
	return 0 <= P.X && 0 <= P.Y && 0 <= P.Z && P.X < GetWidth() && P.Y < GetWidth() && P.Z < GetWidth();
}

FIntVector ValueOctree::GlobalToLocal(FIntVector globalPosition)
{
	return FIntVector(globalPosition.X - (Position.X - GetWidth() / 2), globalPosition.Y - (Position.Y - GetWidth() / 2), globalPosition.Z - (Position.Z - GetWidth() / 2));
}

FIntVector ValueOctree::LocalToGlobal(FIntVector localPosition)
{
	return FIntVector(localPosition.X + (Position.X - GetWidth() / 2), localPosition.Y + (Position.Y - GetWidth() / 2), localPosition.Z + (Position.Z - GetWidth() / 2));
}



void ValueOctree::CreateChilds()
{
	check(IsLeaf());
	int d = GetWidth() / 4;
	Childs[0] = new ValueOctree(Position + FIntVector(-d, -d, -d), Depth - 1);
	Childs[1] = new ValueOctree(Position + FIntVector(+d, -d, -d), Depth - 1);
	Childs[2] = new ValueOctree(Position + FIntVector(-d, +d, -d), Depth - 1);
	Childs[3] = new ValueOctree(Position + FIntVector(+d, +d, -d), Depth - 1);
	Childs[4] = new ValueOctree(Position + FIntVector(-d, -d, +d), Depth - 1);
	Childs[5] = new ValueOctree(Position + FIntVector(+d, -d, +d), Depth - 1);
	Childs[6] = new ValueOctree(Position + FIntVector(-d, +d, +d), Depth - 1);
	Childs[7] = new ValueOctree(Position + FIntVector(+d, +d, +d), Depth - 1);
	bIsLeaf = false;
}