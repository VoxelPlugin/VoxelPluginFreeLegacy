#include "VoxelChunkStruct.h"
#include "VoxelChunk.h"

VoxelChunkStruct::VoxelChunkStruct(AVoxelChunk* chunk) : Depth(chunk->GetDepth()), Chunk(chunk)
{
	check(chunk);

	int Width = 16 << Depth;
	ChunkHasHigherRes[XMin] = chunk->HasChunkHigherRes(-Width, 0, 0);
	ChunkHasHigherRes[XMax] = chunk->HasChunkHigherRes(Width, 0, 0);
	ChunkHasHigherRes[YMin] = chunk->HasChunkHigherRes(0, -Width, 0);
	ChunkHasHigherRes[YMax] = chunk->HasChunkHigherRes(0, Width, 0);
	ChunkHasHigherRes[ZMin] = chunk->HasChunkHigherRes(0, 0, -Width);
	ChunkHasHigherRes[ZMax] = chunk->HasChunkHigherRes(0, 0, Width);

	int Step = 1 << Depth;
	for (int z = -1; z < 18; z++)
	{
		for (int y = -1; y < 18; y++)
		{
			for (int x = -1; x < 18; x++)
			{
				Values[x + 1][y + 1][z + 1] = chunk->GetValue(Step * x, Step * y, Step * z);
			}
		}
	}

	// TODO: improve performance
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			for (int k = 0; k < 16; k++)
			{
				for (int l = 0; l < 10; l++)
				{
					Cache2D[i][j][k][l] = -1;
				}
			}
		}
	}
}

signed char VoxelChunkStruct::GetValue(int x, int y, int z)
{
	int Step = 1 << Depth;

	if (x % Step == 0 && y % Step == 0 && z % Step == 0)
	{
		x /= Step;
		y /= Step;
		z /= Step;
		check(-1 <= x && x < 18);
		check(-1 <= y && y < 18);
		check(-1 <= z && z < 18);
		return Values[x + 1][y + 1][z + 1];
	}
	else
	{
		return Chunk->GetValue(x, y, z);
	}
}

FColor VoxelChunkStruct::GetColor(int x, int y, int z)
{
	return Chunk->GetColor(x, y, z);
}

void VoxelChunkStruct::SaveVertex(int x, int y, int z, short edgeIndex, int index, VertexProperties properties)
{
	auto NewCache = NewCacheIs1 ? Cache1 : Cache2;
	check(0 <= 1 + x && 1 + x < 18);
	check(0 <= 1 + y && 1 + y < 18);
	check(0 <= edgeIndex && edgeIndex < 4);
	NewCache[1 + x][1 + y][edgeIndex] = index;


	const int GlobalX = 2;
	const int GlobalY = 1;
	const int GlobalZ = 3;
	const int LocalX = 8;
	const int LocalY = 9;

	int Direction;
	if (properties.IsNearXMin && edgeIndex != GlobalX && 0 <= y && y < 16 && 0 <= z && z < 16)
	{
		Direction = XMin;
	}
	else if (properties.IsNearXMax && edgeIndex != GlobalX && 0 <= y && y < 16 && 0 <= z && z < 16)
	{
		Direction = XMax;
	}
	else if (properties.IsNearYMin && edgeIndex != GlobalY && 0 <= x && x < 16 && 0 <= z && z < 16)
	{
		Direction = YMin;
	}
	else if (properties.IsNearYMax && edgeIndex != GlobalY && 0 <= x && x < 16 && 0 <= z && z < 16)
	{
		Direction = YMax;
	}
	else if (properties.IsNearZMin && edgeIndex != GlobalZ && 0 <= y && y < 16 && 0 <= x && x < 16)
	{
		Direction = ZMin;
	}
	else if (properties.IsNearZMax && edgeIndex != GlobalZ && 0 <= y && y < 16 && 0 <= x && x < 16)
	{
		Direction = ZMax;
	}
	else
	{
		return;
	}

	if (!ChunkHasHigherRes[Direction])
	{
		return;
	}

	int LocalEdgeIndex;

	FIntVector LocalPos;

	switch (Direction)
	{
	case XMin:
		LocalPos = FIntVector(y, z, x);
		break;
	case XMax:
		LocalPos = FIntVector(15 - y, z, 15 - x);
		break;
	case YMin:
		LocalPos = FIntVector(15 - x, z, y);
		break;
	case YMax:
		LocalPos = FIntVector(15 - x, 15 - z, 15 - y);
		break;
	case ZMin:
		LocalPos = FIntVector(15 - y, x, z);
		break;
	case ZMax:
		LocalPos = FIntVector(15 - y, 15 - x, 15 - z);
		break;
	default:
		check(false);
	}

	switch (Direction)
	{
	case XMin:
		//return FIntVector(y, z, x);
		if (edgeIndex == GlobalY)
		{
			LocalEdgeIndex = LocalX;
		}
		else if (edgeIndex == GlobalZ)
		{
			LocalEdgeIndex = LocalY;
		}
		else
		{
			check(false);
		}
		break;
	case XMax:
		//return FIntVector(width - y, z, width - x);
		if (edgeIndex == GlobalY)
		{
			LocalEdgeIndex = LocalX;
		}
		else if (edgeIndex == GlobalZ)
		{
			LocalPos.X--;
			LocalEdgeIndex = LocalY;
		}
		else
		{
			check(false);
		}
		break;
	case YMin:
		//return FIntVector(width - x, z, y);
		if (edgeIndex == GlobalX)
		{
			LocalEdgeIndex = LocalX;
		}
		else if (edgeIndex == GlobalZ)
		{
			LocalPos.X--;
			LocalEdgeIndex = LocalY;
		}
		else
		{
			check(false);
		}
		break;
	case YMax:
		//return FIntVector(width - x, width - z, width - y);
		if (edgeIndex == GlobalX)
		{
			LocalPos.Y--;
			LocalEdgeIndex = LocalX;
		}
		else if (edgeIndex == GlobalZ)
		{
			LocalPos.X--;
			LocalEdgeIndex = LocalY;
		}
		else
		{
			check(false);
		}
		break;
	case ZMin:
		//return FIntVector(width - y, x, z);
		if (edgeIndex == GlobalY)
		{
			LocalEdgeIndex = LocalX;
		}
		else if (edgeIndex == GlobalX)
		{
			LocalPos.X--;
			LocalEdgeIndex = LocalY;
		}
		else
		{
			check(false);
		}
		break;
	case ZMax:
		//return FIntVector(width - y, width - x, width - z);
		if (edgeIndex == GlobalY)
		{
			LocalPos.Y--;
			LocalEdgeIndex = LocalX;
		}
		else if (edgeIndex == GlobalX)
		{
			LocalPos.X--;
			LocalEdgeIndex = LocalY;
		}
		else
		{
			check(false);
		}
		break;
	default:
		check(false);
	}

	if (LocalPos.X < 0 || LocalPos.X > 15 || LocalPos.Y < 0 || LocalPos.Y > 15)
	{
		//check(false);
		return;
	}

	int i = Cache2D[Direction][LocalPos.X][LocalPos.Y][LocalEdgeIndex];
	check(i >= 0);
	EquivalenceList.push_front(index);
	EquivalenceList.push_front(i);

	if (LocalEdgeIndex == LocalX)
	{
		int j = Cache2D[Direction][LocalPos.X][LocalPos.Y][3];
		int k = Cache2D[Direction][LocalPos.X][LocalPos.Y][4];
		int l = Cache2D[Direction][LocalPos.X][LocalPos.Y][1];
		if (j != -1)
		{
			NormalsEquivalenceList.push_front(index);
			NormalsEquivalenceList.push_front(j);
		}
		if (k != -1)
		{
			NormalsEquivalenceList.push_front(index);
			NormalsEquivalenceList.push_front(k);
		}
		if (l != -1)
		{
			NormalsEquivalenceList.push_front(index);
			NormalsEquivalenceList.push_front(l);
		}
	}
	else if (LocalEdgeIndex == LocalY)
	{
		int j = Cache2D[Direction][LocalPos.X][LocalPos.Y][5];
		int k = Cache2D[Direction][LocalPos.X][LocalPos.Y][6];
		int l = Cache2D[Direction][LocalPos.X][LocalPos.Y][2];
		if (j != -1)
		{
			NormalsEquivalenceList.push_front(index);
			NormalsEquivalenceList.push_front(j);
		}
		if (k != -1)
		{
			NormalsEquivalenceList.push_front(index);
			NormalsEquivalenceList.push_front(k);
		}
		if (l != -1)
		{
			NormalsEquivalenceList.push_front(index);
			NormalsEquivalenceList.push_front(l);
		}
	}

}

int VoxelChunkStruct::LoadVertex(int x, int y, int z, short direction, short edgeIndex)
{
	auto NewCache = NewCacheIs1 ? Cache1 : Cache2;
	auto OldCache = NewCacheIs1 ? Cache2 : Cache1;

	bool xIsDifferent = direction & 0x01;
	bool yIsDifferent = direction & 0x02;
	bool zIsDifferent = direction & 0x04;
	check(0 <= 1 + x - xIsDifferent && 1 + x - xIsDifferent < 18);
	check(0 <= 1 + y - yIsDifferent && 1 + y - yIsDifferent < 18);
	check(0 <= edgeIndex && edgeIndex < 4);
	return (zIsDifferent ? OldCache : NewCache)[1 + x - xIsDifferent][1 + y - yIsDifferent][edgeIndex];
}

int VoxelChunkStruct::GetDepth()
{
	return Depth;
}

bool VoxelChunkStruct::IsNormalOnly(FVector vertex)
{
	int Width = 16 << Depth;
	return vertex.X < 0 || vertex.Y < 0 || vertex.Z < 0 || vertex.X > Width || vertex.Y > Width || vertex.Z > Width;
}

signed char VoxelChunkStruct::GetValue2D(int x, int y)
{
	FIntVector Pos = TransformPosition(x, y, 0, CurrentDirection);
	return GetValue(Pos.X, Pos.Y, Pos.Z);
}

FColor VoxelChunkStruct::GetColor2D(int x, int y)
{
	FIntVector Pos = TransformPosition(x, y, 0, CurrentDirection);
	return GetColor(Pos.X, Pos.Y, Pos.Z);
}

void VoxelChunkStruct::SaveVertex2D(int x, int y, short edgeIndex, int index)
{
	check(0 <= CurrentDirection && CurrentDirection < 7);
	check(0 <= x && 0 <= y && x < 16 && y < 16);
	check(0 <= edgeIndex && edgeIndex < 10);
	Cache2D[CurrentDirection][x][y][edgeIndex] = index;
}

int VoxelChunkStruct::LoadVertex2D(int x, int y, short direction, short edgeIndex)
{
	bool xIsDifferent = direction & 0x01;
	bool yIsDifferent = direction & 0x02;

	check(0 <= CurrentDirection && CurrentDirection < 7);
	check(0 <= x - xIsDifferent && x - xIsDifferent < 16);
	check(0 <= y - yIsDifferent && y - yIsDifferent < 16);
	check(0 <= edgeIndex && edgeIndex < 10);
	return Cache2D[CurrentDirection][x - xIsDifferent][y - yIsDifferent][edgeIndex];
}

FIntVector VoxelChunkStruct::TransformPosition(int x, int y, int z, TransitionDirection direction)
{
	int width = 16 << Depth;
	switch (direction)
	{
	case XMin:
		return FIntVector(z, x, y);
	case XMax:
		return FIntVector(width - z, width - x, y);
	case YMin:
		return FIntVector(width - x, z, y);
	case YMax:
		return FIntVector(width - x, width - z, width - y);
	case ZMin:
		return FIntVector(y, width - x, z);
	case ZMax:
		return FIntVector(width - y, width - x, width - z);
	default:
		check(false);
		return FIntVector::ZeroValue;
	}
}

FVector VoxelChunkStruct::TransformPosition(FVector vertex, TransitionDirection direction)
{
	int width = 16 << Depth;
	float x = vertex.X;
	float y = vertex.Y;
	float z = vertex.Z;
	switch (direction)
	{
	case XMin:
		return FVector(z, x, y);
	case XMax:
		return FVector(width - z, width - x, y);
	case YMin:
		return FVector(width - x, z, y);
	case YMax:
		return FVector(width - x, width - z, width - y);
	case ZMin:
		return FVector(y, width - x, z);
	case ZMax:
		return FVector(width - y, width - x, width - z);
	default:
		check(false);
		return FVector(0, 0, 0);
	}
}

FBoolVector VoxelChunkStruct::TransformPosition(FBoolVector vertex, TransitionDirection direction)
{
	int width = 16 << Depth;
	switch (direction)
	{
	case XMin:
		return FBoolVector(vertex.Z, vertex.X, vertex.Y);
	case XMax:
		return FBoolVector(vertex.Z, vertex.X, vertex.Y);
	case YMin:
		return FBoolVector(vertex.X, vertex.Z, vertex.Y);
	case YMax:
		return FBoolVector(vertex.X, vertex.Z, vertex.Y);
	case ZMin:
		return FBoolVector(vertex.Y, vertex.X, vertex.Z);
	case ZMax:
		return FBoolVector(vertex.Y, vertex.X, vertex.Z);
	default:
		check(false);
		return FBoolVector(false, false, false);
	}
}

FIntVector VoxelChunkStruct::InverseTransformPosition(int x, int y, int z, TransitionDirection direction)
{
	int width = 16 << Depth;
	switch (direction)
	{
	case XMin:
		// return FIntVector(z, x, y);
		return FIntVector(y, z, x);
	case XMax:
		// return FIntVector(width - z, width - x, y);
		return FIntVector(width - y, z, width - x);
	case YMin:
		// return FIntVector(width - x, z, y);
		return FIntVector(width - x, z, y);
	case YMax:
		// return FIntVector(width - x, width - z, width - y);
		return FIntVector(width - x, width - z, width - y);
	case ZMin:
		// return FIntVector(y, width - x, z);
		return FIntVector(width - y, x, z);
	case ZMax:
		// return FIntVector(width - y, width - x, width - z);
		return FIntVector(width - y, width - x, width - z);
	default:
		check(false);
		return FIntVector::ZeroValue;
	}
}
