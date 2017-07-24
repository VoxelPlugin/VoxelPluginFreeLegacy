#include "World.h"
#include "Chunk.h"
#include "EngineGlobals.h"
#include "Engine.h"

World::World(int size) : Size(size * 16 + 1), SizeInChunks(size), Chunks(SizeInChunks*SizeInChunks*SizeInChunks), values(Size*Size*Size)
{
	for (int x = 0; x < SizeInChunks; x++)
	{
		for (int y = 0; y < SizeInChunks; y++)
		{
			for (int z = 0; z < SizeInChunks; z++)
			{
				Chunks[x + SizeInChunks*y + SizeInChunks*SizeInChunks*z] = new Chunk(this, 16 * x, 16 * y, 16 * z);
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

void World::Plane()
{
	for (int x = 0; x < Size; x++)
	{
		for (int y = 0; y < Size; y++)
		{
			for (int z = 0; z < Size; z++)
			{
				values[x + Size*y + Size*Size*z] = (z > Size / 2) ? 1 : -1;
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
				values[x + Size*y + Size*Size*z] = (FVector(x - Size / 2, y - Size / 2, z - Size / 2).Size() - 5.0f) / (FMath::Sqrt(3) * Size / 2) * 128;
			}
		}
	}
}


char World::GetValue(int x, int y, int z)
{
	if (!IsInWorld(x, y, z))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Invalid args: %d, %d, %d"), x, y, z));
	}
	return values[x + Size*y + Size*Size*z];
}

void World::Add(int x, int y, int z)
{
	if (!IsInWorld(x, y, z))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Invalid args: %d, %d, %d"), x, y, z));
	}
	values[x + Size*y + Size*Size*z] -= 1;
}

void World::Remove(int x, int y, int z)
{
	if (!IsInWorld(x, y, z))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Invalid args: %d, %d, %d"), x, y, z));
	}
	values[x + Size*y + Size*Size*z] += 1;
}

bool World::IsInWorld(int x, int y, int z)
{
	return x < Size && y < Size && z < Size && x >= 0 && y >= 0 && z >= 0;
}