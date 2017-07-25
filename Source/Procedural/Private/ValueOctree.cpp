#include "ValueOctree.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "EngineGlobals.h"
#include "Engine.h"

ValueOctree::ValueOctree(int x, int y, int z, int depth) : X(x), Y(y), Z(z), Depth(depth), bIsDirty(false), bIsLeaf(false)
{

}

ValueOctree::~ValueOctree()
{

}



bool ValueOctree::CreateChilds()
{
	if (!IsLeaf())
	{
		int d = GetWidth() / 2;
		Childs[0] = new ValueOctree(X, Y, Z, Depth - 1);
		Childs[1] = new ValueOctree(X + d, Y, Z, Depth - 1);
		Childs[2] = new ValueOctree(X, Y + d, Z, Depth - 1);
		Childs[3] = new ValueOctree(X + d, Y + d, Z, Depth - 1);
		Childs[4] = new ValueOctree(X, Y, Z + d, Depth - 1);
		Childs[5] = new ValueOctree(X + d, Y, Z + d, Depth - 1);
		Childs[6] = new ValueOctree(X, Y + d, Z + d, Depth - 1);
		Childs[7] = new ValueOctree(X + d, Y + d, Z + d, Depth - 1);
		return true;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, TEXT("Error: Cannot create childs: IsLeaf"));
		return false;
	}
}

inline int ValueOctree::GetWidth()
{
	return 16 << Depth;
}

void ValueOctree::CreateTree(FVector cameraPosition)
{
	float distanceToCamera = (FVector(X, Y, Z) - cameraPosition).Size();

	if (distanceToCamera > GetWidth() || Depth == 0)
	{
		bIsLeaf = true;
	}
	else
	{
		if (CreateChilds())
		{
			for (int i = 0; i < 8; i++)
			{
				Childs[i]->CreateTree(cameraPosition);
			}
		}
	}
}

ValueOctree* ValueOctree::GetLeaf(int x, int y, int z)
{
	if (IsLeaf())
	{
		return this;
	}
	else
	{
		// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
		return Childs[((x >= X + GetWidth() / 2) ? 1 : 0) + ((y >= Y + GetWidth() / 2) ? 2 : 0) + ((z >= Z + GetWidth() / 2) ? 4 : 0)];
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

signed char ValueOctree::GetValue(int x, int y, int z)
{
	if (IsInChunk(x, y, z))
	{
		if (IsDirty())
		{
			int w = GetWidth();
			return Values[x - X + w * (y - Y) + w * w * (z - Z)];
		}
		else
		{
			return (z > 8) ? 100 : -100;
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Error: Not in chunk (%d, %d, %d)"), x, y, z));
		return (z > 10) ? 1 : -1;
	}
}

void ValueOctree::SetValue(int x, int y, int z, signed char value)
{
	if (IsInChunk(x, y, z))
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
						Values[i + w * j + w * w * k] = GetValue(X + i, Y + j, Z + k);
					}
				}
			}
			bIsDirty = true;
		}
		Values[x - X + w* (y - Y) + w * w * (z - Z)] = value;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Error: Not in chunk (%d, %d, %d)"), x, y, z));
	}
}

bool ValueOctree::IsInChunk(int x, int y, int z)
{
	int w = GetWidth();
	return x >= X && y >= Y && z >= Z && x < w + X  && y < w + Y  && z < w + Z;
}