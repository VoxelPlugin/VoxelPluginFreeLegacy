#include "VoxelChunkStruct.h"
#include "VoxelChunk.h"

VoxelChunkStruct::VoxelChunkStruct(AVoxelChunk* Chunk) : Depth(Chunk->GetDepth()), Chunk(Chunk), Width(Chunk->Width()), Step(1 << Depth)
{
	check(Chunk);

	for (int z = -1; z < 18; z++)
	{
		for (int y = -1; y < 18; y++)
		{
			for (int x = -1; x < 18; x++)
			{
				Values[x + 1][y + 1][z + 1] = Chunk->GetValue(Step * x, Step * y, Step * z);
			}
		}
	}

	for (int i = 0; i < 6; i++)
	{
		ChunkHasHigherRes[i] = Chunk->ChunkHasHigherRes[i];
	}

	if (Depth != 0)
	{
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
}

signed char VoxelChunkStruct::GetValue(int X, int Y, int Z)
{
	if (X % Step == 0 && Y % Step == 0 && Z % Step == 0)
	{
		X /= Step;
		Y /= Step;
		Z /= Step;
		check(-1 <= X && X < 18);
		check(-1 <= Y && Y < 18);
		check(-1 <= Z && Z < 18);
		return Values[X + 1][Y + 1][Z + 1];
	}
	else
	{
		return Chunk->GetValue(X, Y, Z);
	}
}

FColor VoxelChunkStruct::GetColor(int X, int Y, int Z)
{
	return Chunk->GetColor(X, Y, Z);
}

void VoxelChunkStruct::SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index, VertexProperties Properties)
{
	auto NewCache = NewCacheIs1 ? Cache1 : Cache2;
	check(0 <= 1 + X && 1 + X < 18);
	check(0 <= 1 + Y && 1 + Y < 18);
	check(0 <= EdgeIndex && EdgeIndex < 4);
	NewCache[1 + X][1 + Y][EdgeIndex] = Index;

	if (Depth != 0)
	{
		const int GlobalX = 2;
		const int GlobalY = 1;
		const int GlobalZ = 3;
		const int LocalX = 8;
		const int LocalY = 9;

		int Direction;
		if (Properties.IsNearXMin && EdgeIndex != GlobalX && 0 <= Y && Y < 16 && 0 <= Z && Z < 16)
		{
			Direction = XMin;
		}
		else if (Properties.IsNearXMax && EdgeIndex != GlobalX && 0 <= Y && Y < 16 && 0 <= Z && Z < 16)
		{
			Direction = XMax;
		}
		else if (Properties.IsNearYMin && EdgeIndex != GlobalY && 0 <= X && X < 16 && 0 <= Z && Z < 16)
		{
			Direction = YMin;
		}
		else if (Properties.IsNearYMax && EdgeIndex != GlobalY && 0 <= X && X < 16 && 0 <= Z && Z < 16)
		{
			Direction = YMax;
		}
		else if (Properties.IsNearZMin && EdgeIndex != GlobalZ && 0 <= Y && Y < 16 && 0 <= X && X < 16)
		{
			Direction = ZMin;
		}
		else if (Properties.IsNearZMax && EdgeIndex != GlobalZ && 0 <= Y && Y < 16 && 0 <= X && X < 16)
		{
			Direction = ZMax;
		}
		else
		{
			return;
		}

		check(0 <= Direction && Direction < 6);

		if (!ChunkHasHigherRes[Direction])
		{
			return;
		}

		int LocalEdgeIndex;

		FIntVector LocalPos;

		switch (Direction)
		{
		case XMin:
			LocalPos = FIntVector(Y, Z, X);
			break;
		case XMax:
			LocalPos = FIntVector(15 - Y, Z, 15 - X);
			break;
		case YMin:
			LocalPos = FIntVector(15 - X, Z, Y);
			break;
		case YMax:
			LocalPos = FIntVector(15 - X, 15 - Z, 15 - Y);
			break;
		case ZMin:
			LocalPos = FIntVector(15 - Y, X, Z);
			break;
		case ZMax:
			LocalPos = FIntVector(15 - Y, 15 - X, 15 - Z);
			break;
		default:
			check(false);
			return;
		}

		switch (Direction)
		{
		case XMin:
			//return FIntVector(Y, Z, X);
			if (EdgeIndex == GlobalY)
			{
				LocalEdgeIndex = LocalX;
			}
			else if (EdgeIndex == GlobalZ)
			{
				LocalEdgeIndex = LocalY;
			}
			else
			{
				check(false);
				return;
			}
			break;
		case XMax:
			//return FIntVector(width - Y, Z, width - X);
			if (EdgeIndex == GlobalY)
			{
				LocalEdgeIndex = LocalX;
			}
			else if (EdgeIndex == GlobalZ)
			{
				LocalPos.X--;
				LocalEdgeIndex = LocalY;
			}
			else
			{
				check(false);
				return;
			}
			break;
		case YMin:
			//return FIntVector(width - X, Z, Y);
			if (EdgeIndex == GlobalX)
			{
				LocalEdgeIndex = LocalX;
			}
			else if (EdgeIndex == GlobalZ)
			{
				LocalPos.X--;
				LocalEdgeIndex = LocalY;
			}
			else
			{
				check(false);
				return;
			}
			break;
		case YMax:
			//return FIntVector(width - X, width - Z, width - Y);
			if (EdgeIndex == GlobalX)
			{
				LocalPos.Y--;
				LocalEdgeIndex = LocalX;
			}
			else if (EdgeIndex == GlobalZ)
			{
				LocalPos.X--;
				LocalEdgeIndex = LocalY;
			}
			else
			{
				check(false);
				return;
			}
			break;
		case ZMin:
			//return FIntVector(width - Y, X, Z);
			if (EdgeIndex == GlobalY)
			{
				LocalEdgeIndex = LocalX;
			}
			else if (EdgeIndex == GlobalX)
			{
				LocalPos.X--;
				LocalEdgeIndex = LocalY;
			}
			else
			{
				check(false);
				return;
			}
			break;
		case ZMax:
			//return FIntVector(width - Y, width - X, width - Z);
			if (EdgeIndex == GlobalY)
			{
				LocalPos.Y--;
				LocalEdgeIndex = LocalX;
			}
			else if (EdgeIndex == GlobalX)
			{
				LocalPos.X--;
				LocalEdgeIndex = LocalY;
			}
			else
			{
				check(false);
				return;
			}
			break;
		default:
			check(false);
			return;
		}

		if (LocalPos.X < 0 || LocalPos.X > 15 || LocalPos.Y < 0 || LocalPos.Y > 15)
		{
			return;
		}

		check(0 <= Direction && Direction <= 6);
		check(0 <= LocalPos.X && LocalPos.X < 16);
		check(0 <= LocalPos.Y && LocalPos.Y < 16);
		check(0 <= LocalEdgeIndex && LocalEdgeIndex < 10);
		int i = Cache2D[Direction][LocalPos.X][LocalPos.Y][LocalEdgeIndex];
		check(i >= 0);
		EquivalenceList.push_front(Index);
		EquivalenceList.push_front(i);

		if (LocalEdgeIndex == LocalX)
		{
			int j = Cache2D[Direction][LocalPos.X][LocalPos.Y][3];
			int k = Cache2D[Direction][LocalPos.X][LocalPos.Y][4];
			int l = Cache2D[Direction][LocalPos.X][LocalPos.Y][1];
			if (j != -1)
			{
				NormalsEquivalenceList.push_front(Index);
				NormalsEquivalenceList.push_front(j);
			}
			if (k != -1)
			{
				NormalsEquivalenceList.push_front(Index);
				NormalsEquivalenceList.push_front(k);
			}
			if (l != -1)
			{
				NormalsEquivalenceList.push_front(Index);
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
				NormalsEquivalenceList.push_front(Index);
				NormalsEquivalenceList.push_front(j);
			}
			if (k != -1)
			{
				NormalsEquivalenceList.push_front(Index);
				NormalsEquivalenceList.push_front(k);
			}
			if (l != -1)
			{
				NormalsEquivalenceList.push_front(Index);
				NormalsEquivalenceList.push_front(l);
			}
		}
	}
}

int VoxelChunkStruct::LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex)
{
	auto NewCache = NewCacheIs1 ? Cache1 : Cache2;
	auto OldCache = NewCacheIs1 ? Cache2 : Cache1;

	bool XIsDifferent = Direction & 0x01;
	bool YIsDifferent = Direction & 0x02;
	bool ZIsDifferent = Direction & 0x04;
	check(0 <= 1 + X - XIsDifferent && 1 + X - XIsDifferent < 18);
	check(0 <= 1 + Y - YIsDifferent && 1 + Y - YIsDifferent < 18);
	check(0 <= EdgeIndex && EdgeIndex < 4);
	return (ZIsDifferent ? OldCache : NewCache)[1 + X - XIsDifferent][1 + Y - YIsDifferent][EdgeIndex];
}

bool VoxelChunkStruct::IsNormalOnly(FVector Vertex)
{
	return Vertex.X < 0 || Vertex.Y < 0 || Vertex.Z < 0 || Vertex.X > Width || Vertex.Y > Width || Vertex.Z > Width;
}

signed char VoxelChunkStruct::GetValue2D(int X, int Y)
{
	FIntVector Position = TransformPosition(X, Y, 0, CurrentDirection);
	return GetValue(Position.X, Position.Y, Position.Z);
}

FColor VoxelChunkStruct::GetColor2D(int X, int Y)
{
	FIntVector Position = TransformPosition(X, Y, 0, CurrentDirection);
	return GetColor(Position.X, Position.Y, Position.Z);
}

void VoxelChunkStruct::SaveVertex2D(int X, int Y, short EdgeIndex, int Index)
{
	check(0 <= CurrentDirection && CurrentDirection < 6);
	check(0 <= X && 0 <= Y && X < 16 && Y < 16);
	check(0 <= EdgeIndex && EdgeIndex < 10);
	Cache2D[CurrentDirection][X][Y][EdgeIndex] = Index;
}

int VoxelChunkStruct::LoadVertex2D(int X, int Y, short Direction, short EdgeIndex)
{
	bool XIsDifferent = Direction & 0x01;
	bool YIsDifferent = Direction & 0x02;

	check(0 <= CurrentDirection && CurrentDirection < 6);
	check(0 <= X - XIsDifferent && X - XIsDifferent < 16);
	check(0 <= Y - YIsDifferent && Y - YIsDifferent < 16);
	check(0 <= EdgeIndex && EdgeIndex < 10);
	return Cache2D[CurrentDirection][X - XIsDifferent][Y - YIsDifferent][EdgeIndex];
}

TransitionDirection VoxelChunkStruct::GetDirection()
{
	return CurrentDirection;
}

FIntVector VoxelChunkStruct::TransformPosition(int X, int Y, int Z, TransitionDirection Direction)
{
	switch (Direction)
	{
	case XMin:
		return FIntVector(Z, X, Y);
	case XMax:
		return FIntVector(Width - Z, Width - X, Y);
	case YMin:
		return FIntVector(Width - X, Z, Y);
	case YMax:
		return FIntVector(Width - X, Width - Z, Width - Y);
	case ZMin:
		return FIntVector(Y, Width - X, Z);
	case ZMax:
		return FIntVector(Width - Y, Width - X, Width - Z);
	default:
		check(false);
		return FIntVector::ZeroValue;
	}
}

FVector VoxelChunkStruct::TransformPosition(FVector Vertex, TransitionDirection Direction)
{
	float X = Vertex.X;
	float Y = Vertex.Y;
	float Z = Vertex.Z;
	switch (Direction)
	{
	case XMin:
		return FVector(Z, X, Y);
	case XMax:
		return FVector(Width - Z, Width - X, Y);
	case YMin:
		return FVector(Width - X, Z, Y);
	case YMax:
		return FVector(Width - X, Width - Z, Width - Y);
	case ZMin:
		return FVector(Y, Width - X, Z);
	case ZMax:
		return FVector(Width - Y, Width - X, Width - Z);
	default:
		check(false);
		return FVector(0, 0, 0);
	}
}

FBoolVector VoxelChunkStruct::TransformPosition(FBoolVector Vertex, TransitionDirection Direction)
{
	switch (Direction)
	{
	case XMin:
		return FBoolVector(Vertex.Z, Vertex.X, Vertex.Y);
	case XMax:
		return FBoolVector(Vertex.Z, Vertex.X, Vertex.Y);
	case YMin:
		return FBoolVector(Vertex.X, Vertex.Z, Vertex.Y);
	case YMax:
		return FBoolVector(Vertex.X, Vertex.Z, Vertex.Y);
	case ZMin:
		return FBoolVector(Vertex.Y, Vertex.X, Vertex.Z);
	case ZMax:
		return FBoolVector(Vertex.Y, Vertex.X, Vertex.Z);
	default:
		check(false);
		return FBoolVector(false, false, false);
	}
}

FIntVector VoxelChunkStruct::InverseTransformPosition(int X, int Y, int Z, TransitionDirection Direction)
{
	switch (Direction)
	{
	case XMin:
		// return FIntVector(Z, X, Y);
		return FIntVector(Y, Z, X);
	case XMax:
		// return FIntVector(Width - Z, Width - X, Y);
		return FIntVector(Width - Y, Z, Width - X);
	case YMin:
		// return FIntVector(Width - X, Z, Y);
		return FIntVector(Width - X, Z, Y);
	case YMax:
		// return FIntVector(Width - X, Width - Z, Width - Y);
		return FIntVector(Width - X, Width - Z, Width - Y);
	case ZMin:
		// return FIntVector(Y, Width - X, Z);
		return FIntVector(Width - Y, X, Z);
	case ZMax:
		// return FIntVector(Width - Y, Width - X, Width - Z);
		return FIntVector(Width - Y, Width - X, Width - Z);
	default:
		check(false);
		return FIntVector::ZeroValue;
	}
}
