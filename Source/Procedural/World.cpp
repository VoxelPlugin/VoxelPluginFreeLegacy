#include "World.h"
#include "Chunk.h"

World::World(int size) : Size(size * Chunk::Size), ChunkSize(size), Level(0), Chunks(ChunkSize*ChunkSize*ChunkSize), values(Size*Size*Size)
{
	for (int x = 0; x < ChunkSize; x++)
	{
		for (int y = 0; y < ChunkSize; y++)
		{
			for (int z = 0; z < ChunkSize; z++)
			{
				Chunks[x + ChunkSize*y + ChunkSize*ChunkSize*z] = new Chunk(this, Chunk::Size*x, Chunk::Size*y, Chunk::Size*z);
			}
		}
	}
}

World::~World()
{
	for (auto it = Chunks.begin(); it != Chunks.end(); ++it)
	{
		delete *it;
	}
}



void World::Randomize()
{
	for (int x = 0; x < Size; x++)
	{
		for (int y = 0; y < Size; y++)
		{
			for (int z = 0; z < Size; z++)
			{
				values[x + Size*y + Size*Size*z] = FMath::FRandRange(-1, 10);
			}
		}
	}
}

void World::Sphere()
{
	for (int x = 0; x < Size; x++)
	{
		for (int y = 0; y < Size; y++)
		{
			for (int z = 0; z < Size; z++)
			{
				values[x + Size*y + Size*Size*z] = -FVector(x - Size / 2, y - Size / 2, z - Size / 2).Size();
			}
		}
	}
}


float World::GetValue(int x, int y, int z)
{
	checkf(x < Size && y < Size && z < Size && x >= 0 && y >= 0 && z >= 0, TEXT("Invalid args: %d, %d, %d"), x, y, z);
	return values[x + Size*y + Size*Size*z];
}